#include "usageTask.hpp"
#include "esp_log.h"

#include <driver/gpio.h>
#include <esp_idf_lib_helpers.h>
#include <esp_timer.h>
#include <ets_sys.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

using namespace Zotbins;

const gpio_num_t PIN_BREAKBEAM = GPIO_NUM_16;

static const char *name = "usageTask";
static const int priority = 1;
static const uint32_t stackSize = 4096;
static TaskHandle_t xTaskToNotify = NULL;
static const int core = 1;

UsageTask::UsageTask(QueueHandle_t &messageQueue)
    : Task(name, priority, stackSize), mMessageQueue(messageQueue)
{
}

void UsageTask::start()
{
    xTaskCreatePinnedToCore(taskFunction, mName, mStackSize, this, mPriority, &xTaskToNotify, core);
}

void UsageTask::taskFunction(void *task)
{
    UsageTask *usageTask = static_cast<UsageTask *>(task);
    usageTask->setup();
    usageTask->loop();
}

void UsageTask::setup()
{
}

void UsageTask::loop()
{
    // From https://www.espressif.com/sites/default/files/documentation/esp32_technical_reference_manual_en.pdf#iomuxgpio
    // GPIO pads 34-39 are input-only.
    ESP_LOGI(name, "Hello from Usage Task"); // init

    // Setup GPIO pin sensors
    gpio_set_direction(DETECT_PIN, GPIO_MODE_INPUT);
    while (1)
    {
        // Read in signal from breakbeam
        int detect = gpio_get_level(PIN_BREAKBEAM);
        // If breakbeam is disconnected
        if (detect == 0)
        {
            while (detect == 0)
            {
                ESP_LOGI(name, "Detecting item");
                detect = gpio_get_level(PIN_BREAKBEAM);
            }
            ESP_LOGI(name, "No longer detected");
            xTaskToNotify = xTaskGetHandle("fullnessTask");
            xTaskNotifyGive(xTaskToNotify);
            vTaskSuspend(NULL);
        }
        ESP_LOGI(name, "Nothing detected");
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
