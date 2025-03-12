#include "usageTask.hpp"
#include "esp_log.h"

#include <driver/gpio.h>
#include <esp_idf_lib_helpers.h>
#include <esp_timer.h>
#include <ets_sys.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "Client.hpp"

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

    int32_t example_weight = 75;
    float distance = 22.5;

    Client::clientPublish("weight", "SENSOR", &example_weight);  
    Client::clientPublish("distance", "SENSOR", &distance);

    while(1){
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    // Setup GPIO pin sensors
    gpio_set_direction(PIN_BREAKBEAM, GPIO_MODE_INPUT);
    while (1)
    {
        if (!beamBroken)
        {
            vTaskSuspend(NULL); // Suspend the task until notified
        }
        bool DETECTED = !gpio_get_level(PIN_BREAKBEAM); // Read in signal from breakbeam
        if (DETECTED) // If breakbeam is disconnected
        {
            ESP_LOGI(name, "Detected item.");
            while (DETECTED)
            {
                DETECTED = !gpio_get_level(PIN_BREAKBEAM);
            }
            ESP_LOGI(name, "Item no longer detected.");
            beamBroken = false;
            xTaskToNotify = xTaskGetHandle("fullnessTask"); 
            xTaskNotifyGive(xTaskToNotify); // once item is no longer detected collect fullness data
            ESP_LOGI(name, "Notified Fullness Task");
            vTaskSuspend(NULL);
        }
        ESP_LOGI(name, "Ready to detect.");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
