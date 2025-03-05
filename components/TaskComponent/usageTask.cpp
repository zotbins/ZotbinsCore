#include "usageTask.hpp"
#include "esp_log.h"

#include <driver/gpio.h>
#include <esp_idf_lib_helpers.h>
#include <esp_timer.h>
#include <ets_sys.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

using namespace Zotbins;

const gpio_num_t PIN_BREAKBEAM = GPIO_NUM_19;

const gpio_config_t PIN_BREAKBEAM_CONFIG = {
    .pin_bit_mask = 0x00080000,
    .mode = GPIO_MODE_INPUT,
    .pull_up_en = GPIO_PULLUP_ENABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_NEGEDGE
};

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
    ESP_LOGI(name, "Configuring interrupt GPIO");
    gpio_install_isr_service(0);
    ESP_ERROR_CHECK(gpio_config(&PIN_BREAKBEAM_CONFIG));
    gpio_isr_handler_add(PIN_BREAKBEAM, breakbeamISR, NULL);
    gpio_intr_enable(PIN_BREAKBEAM);
}

void UsageTask::loop()
{
    // From https://www.espressif.com/sites/default/files/documentation/esp32_technical_reference_manual_en.pdf#iomuxgpio
    // GPIO pads 34-39 are input-only.
    // ESP_LOGI(name, "Hello from Usage Task"); // init

    ESP_LOGI(name, "Starting usage loop...");
    while (1)
    {
        ESP_LOGI(name, "\n--------------------Suspending usage task until beam is broken...-------------------\n"); // no if statement needed, will resume from here and complete one iteration then suspend again.
        vTaskSuspend(NULL); // Suspend the task until notified

        bool DETECTED = !gpio_get_level(PIN_BREAKBEAM); // Read in signal from breakbeam
        if (DETECTED) // If breakbeam is disconnected
        {
            ESP_LOGI(name, "\n--------------------Detected item.-------------------\n");
            while (DETECTED)
            {
                DETECTED = !gpio_get_level(PIN_BREAKBEAM);
            }
            ESP_LOGI(name, "Item no longer detected.");
            beamBroken = false;
            xTaskToNotify = xTaskGetHandle("fullnessTask"); 
            xTaskNotifyGive(xTaskToNotify); // once item is no longer detected collect fullness data

            vTaskSuspend(NULL);
        }
        ESP_LOGI(name, "Ready to detect.");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
