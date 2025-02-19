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
static SemaphoreHandle_t usageSem;

const gpio_config_t PIN_BREAKBEAM_CONFIG = {
    .pin_bit_mask = 0x00010000,
    .mode = GPIO_MODE_INPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_NEGEDGE
};

static void IRAM_ATTR breakbeam_isr(void *param)
{
    xSemaphoreGiveFromISR(usageSem, nullptr);
}

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
    // Create semaphore
    usageSem = xSemaphoreCreateBinary();
    
    // Configure gpio for interrupt
    ESP_ERROR_CHECK(gpio_config(&PIN_BREAKBEAM_CONFIG));
    ESP_ERROR_CHECK(gpio_install_isr_service(ESP_INTR_FLAG_LEVEL1));
    ESP_ERROR_CHECK(gpio_isr_handler_add(DETECT_PIN, breakbeam_isr, NULL));
}

void UsageTask::loop()
{
    // From https://www.espressif.com/sites/default/files/documentation/esp32_technical_reference_manual_en.pdf#iomuxgpio
    // GPIO pads 34-39 are input-only.
    ESP_LOGI(name, "Hello from Usage Task"); // init

    // Setup GPIO pin sensors
    ESP_ERROR_CHECK(gpio_set_direction(DETECT_PIN, GPIO_MODE_INPUT));
    for (;;)
    {
        if (xSemaphoreTake(usageSem, portMAX_DELAY) == pdTRUE)
        {
            // Read in signal from breakbeam
            int detect = gpio_get_level(DETECT_PIN);
            if (detect == 1)
            { // If breakbeam is disconnected
                ESP_LOGI(name, "Detecting item");
            }
            else
            {
                ESP_LOGI(name, "No longer detected");
            }
        }
    }
    vTaskDelete(NULL);
}
