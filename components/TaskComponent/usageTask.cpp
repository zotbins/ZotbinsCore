#include "usageTask.hpp"
#include "esp_log.h"

#include <driver/gpio.h>
#include <esp_idf_lib_helpers.h>
#include <esp_timer.h>
#include <ets_sys.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "Client.hpp"
#include "Serialize.hpp"

using namespace Zotbins;

// ESP32-CAM is 16, WROVER is 18

#if defined(SENSOR)
    const gpio_num_t PIN_BREAKBEAM = GPIO_NUM_18;
#elif defined(CAMERA)
    const gpio_num_t PIN_BREAKBEAM = GPIO_NUM_14; // PIN 16 DOES NOT WORK ON THE CAM FOR INTERRUPTS
#endif

const gpio_config_t PIN_BREAKBEAM_CONFIG = {
    .pin_bit_mask = (1ULL << PIN_BREAKBEAM),
    .mode = GPIO_MODE_INPUT,
    .pull_up_en = GPIO_PULLUP_ENABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_NEGEDGE
};

static const char *name = "usageTask";
static const int priority = 1;
static const uint32_t stackSize = 4096;

static TaskHandle_t usageHandle = NULL;
static TaskHandle_t xTaskToNotify = NULL;
static const int core = 1;

static bool DETECTED = false;

UsageTask::UsageTask(QueueHandle_t &messageQueue)
    : Task(name, priority, stackSize), mMessageQueue(messageQueue)
{
}

void IRAM_ATTR breakbeamISR(void *arg)
{
    DETECTED = true;
    vTaskResume(usageHandle);
}

void UsageTask::start()
{
    xTaskCreatePinnedToCore(taskFunction, mName, mStackSize, this, mPriority, &usageHandle, core);
}

void UsageTask::taskFunction(void *task)
{
    // ESP_LOGI(name, "Usage Task Function pinned to core.");
    UsageTask *usageTask = static_cast<UsageTask *>(task);
    usageTask->setup();
    usageTask->loop();
}

void UsageTask::setup()
{
    // ESP_LOGI(name, "Running usage setup.");
    ESP_ERROR_CHECK(gpio_config(&PIN_BREAKBEAM_CONFIG)); // NECESSARY FOR SOME PINS!!
    // ESP_LOGI(name, "1");
    ESP_ERROR_CHECK(gpio_install_isr_service(0));
    // ESP_LOGI(name, "2");
    ESP_ERROR_CHECK(gpio_isr_handler_add(PIN_BREAKBEAM, breakbeamISR, NULL));
    // ESP_LOGI(name, "3");
    ESP_ERROR_CHECK(gpio_intr_enable(PIN_BREAKBEAM));
    // ESP_LOGI(name, "Usage setup complete.");
}

void UsageTask::loop()
{
    // From https://www.espressif.com/sites/default/files/documentation/esp32_technical_reference_manual_en.pdf#iomuxgpio
    // GPIO pads 34-39 are input-only.
    ESP_LOGI(name, "Hello from Usage Task"); // init\=

    usage = 0;

    while (1)
    {
        // Double check breakbeam is broken and bool has tracked that.
        if (DETECTED)
        {
            ESP_LOGI(name, "Detected item.");

            // Breakbeam is broken by an object. Halt until the object leaves the breakbeam's path.
            while (DETECTED)
            {
                DETECTED = !gpio_get_level(PIN_BREAKBEAM);
            }

            ESP_LOGI(name, "Item no longer detected.");

            #if defined(CAMERA)                
                // take picture
                ESP_LOGI(name, "Notifying Camera Task.");
                xTaskToNotify = xTaskGetHandle("cameraTask"); 
                xTaskNotifyGive(xTaskToNotify); // once item is no longer detected collect image data
                ESP_LOGI(name, "Notified Camera Task");

            #elif defined(SENSOR)
                // increment and publish usage data
                ESP_LOGI(name, "Incrementing usage: %i", usage);
                usage += 1;
                Client::clientPublish("usage", static_cast<void*>(&usage));
            
                xTaskToNotify = xTaskGetHandle("fullnessTask"); 
                xTaskNotifyGive(xTaskToNotify); // once item is no longer detected collect fullness data
                ESP_LOGI(name, "Notified Fullness Task");

            #endif

            DETECTED = false; // Reset variable.
            vTaskSuspend(NULL); // Suspend task until next interrupt.

        }

        else {

            ESP_LOGI(name, "Ready to detect.");
            vTaskSuspend(NULL); // Suspend task until next interrupt.

        }
        vTaskDelay(100 /portTICK_PERIOD_MS);
    }
}
