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

// MAKE SURE FOR BOTH ESP32WROVER AND CAM THAT THEY SHARE COMMON GROUND PIN
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
    // ESP_ERROR_CHECK(gpio_install_isr_service(0));
    // ESP_LOGI(name, "2");
    // ESP_ERROR_CHECK(gpio_isr_handler_add(PIN_BREAKBEAM, breakbeamISR, NULL));
    // ESP_LOGI(name, "3");
    // ESP_ERROR_CHECK(gpio_intr_enable(PIN_BREAKBEAM));
    // ESP_LOGI(name, "Usage setup complete.");
}

bool debounce = false;
void UsageTask::loop()
{
    // From https://www.espressif.com/sites/default/files/documentation/esp32_technical_reference_manual_en.pdf#iomuxgpio
    // GPIO pads 34-39 are input-only.
    ESP_LOGI(name, "Hello from Usage Task"); // init\=
    usage = 0;
    while (1)
    {
        if (debounce){
            return;
        }
        // Double check breakbeam is broken and bool has tracked that.
        DETECTED = gpio_get_level(PIN_BREAKBEAM);
        // printf("I AM RUNNING\n");
        if (DETECTED)
        {
            debounce = true;
            #if defined(CAMERA)
                // take picture
                ESP_LOGI(name, "Notifying servo Task.");
                xTaskToNotify = xTaskGetHandle("servoTask");
                xTaskNotifyGive(xTaskToNotify); // once item is no longer detected collect image data
                ulTaskNotifyTake(pdTRUE, (TickType_t)portMAX_DELAY); 
                ESP_LOGI(name, "Notified servo Task");
            #elif defined(SENSOR)
                // increment and publish usage data
                ESP_LOGI(name, "Incrementing usage: %i", usage);
                usage += 1;
                vTaskDelay(10000 /portTICK_PERIOD_MS);
                Client::clientPublish("usage", static_cast<void*>(&usage));
                xTaskToNotify = xTaskGetHandle("fullnessTask");
                xTaskNotifyGive(xTaskToNotify); // once item is no longer detected collect fullness data
                vTaskSuspend(NULL); // Suspend task until next interrupt.
                ESP_LOGI(name, "Notified Fullness Task");
            #endif
            // vTaskSuspend(NULL); // Suspend task until next interrupt.
            // ulTaskNotifyTake(pdTRUE, (TickType_t)portMAX_DELAY); 
            debounce = false;
            DETECTED = false; // Reset variable.
        }
        else {
            // ESP_LOGI(name, "Ready to detect.");
            // vTaskSuspend(NULL); // Suspend task until next interrupt.
        }
        vTaskDelay(100 /portTICK_PERIOD_MS);
    }
}