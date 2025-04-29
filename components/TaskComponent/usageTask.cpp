#include "usageTask.hpp"
#include "esp_log.h"

#include <driver/gpio.h>
#include <esp_idf_lib_helpers.h>
#include <esp_timer.h>
#include <ets_sys.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

using namespace Zotbins;

const gpio_num_t PIN_BREAKBEAM = GPIO_NUM_18;

static const char *name = "usageTask";
static const int priority = 1;
static const uint32_t stackSize = 4096;
static TaskHandle_t usageHandle = NULL;
static const int core = 1;
static bool beamBroken = false;
static TaskHandle_t xTaskToNotify = NULL;

UsageTask::UsageTask(QueueHandle_t &messageQueue)
    : Task(name, priority, stackSize), mMessageQueue(messageQueue)
{
}

void IRAM_ATTR breakbeamISR(void *arg)
{
    beamBroken = true;
    vTaskResume(usageHandle);
}

void UsageTask::start()
{
    xTaskCreatePinnedToCore(taskFunction, mName, mStackSize, this, mPriority, &usageHandle, core);
}

void UsageTask::taskFunction(void *task)
{
    UsageTask *usageTask = static_cast<UsageTask *>(task);
    usageTask->setup();
    usageTask->loop();
}

void UsageTask::setup()
{
    gpio_install_isr_service(0);
    gpio_set_direction(PIN_BREAKBEAM, GPIO_MODE_INPUT);
    gpio_set_intr_type(PIN_BREAKBEAM, GPIO_INTR_NEGEDGE); // Falling edge interrupt
    gpio_isr_handler_add(PIN_BREAKBEAM, breakbeamISR, NULL);
    gpio_intr_enable(PIN_BREAKBEAM);
}

void UsageTask::loop()
{
    // From https://www.espressif.com/sites/default/files/documentation/esp32_technical_reference_manual_en.pdf#iomuxgpio
    // GPIO pads 34-39 are input-only.
    ESP_LOGI(name, "Hello from Usage Task"); // init

    // Setup GPIO pin sensors
    gpio_set_direction(PIN_BREAKBEAM, GPIO_MODE_INPUT);
    while (1)
    {
        if (DETECTED) {
            ESP_LOGI(name, "Detected item.");
            
            try {
                int waitTimeMs = 0;
                const int maxWaitMs = 5000; // 5 seconds timeout

                while (DETECTED && waitTimeMs < maxWaitMs)
                {
                    vTaskDelay(100 / portTICK_PERIOD_MS);
                    waitTimeMs += 100;
                    DETECTED = !gpio_get_level(PIN_BREAKBEAM);
                }

                if (waitTimeMs >= maxWaitMs) {
                    throw std::runtime_error("Breakbeam stuck in 'broken' state for too long");
                }

                ESP_LOGI(name, "Item no longer detected.");
            } catch (const std::exception& e) {
                ESP_LOGE(name, "Breakbeam Sensor Error: %s", e.what());
                beamBroken = false;
                continue;
            }

            beamBroken = false;
            xTaskToNotify = xTaskGetHandle("fullnessTask"); 
            xTaskNotifyGive(xTaskToNotify);

            vTaskSuspend(NULL);
    }
}