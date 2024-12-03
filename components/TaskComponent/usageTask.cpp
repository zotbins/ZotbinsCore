#include "usageTask.hpp"
#include "esp_log.h"

#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_timer.h>
#include <ets_sys.h>
#include <esp_idf_lib_helpers.h>

using namespace Zotbins;

static const char *name = "usageTask";
static const int priority = 1;
static const uint32_t stackSize = 4096;

UsageTask::UsageTask(QueueHandle_t &messageQueue)
    : Task(name, priority, stackSize), mMessageQueue(messageQueue)
{
}

void UsageTask::start()
{
    xTaskCreate(taskFunction, mName, mStackSize, this, mPriority, nullptr);
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
    // Use I/O ports
    ESP_LOGI(name, "Hello from Usage Task");

    // Setup GPIO pin sensors
    gpio_num_t DETECT_PIN = GPIO_NUM_16;
    gpio_set_level(DETECT_PIN, 0);
    gpio_set_direction(DETECT_PIN, GPIO_MODE_INPUT);
    while (1)
    {
        // Read in signal from breakbeam
        int detect = gpio_get_level(DETECT_PIN);
        if (detect == 0) {  // If breakbeam is disconnected
            ESP_LOGI(name, "Detecting item");
        } else {
            ESP_LOGI(name, "No longer detected");
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay for 1000 milliseconds
    }
}