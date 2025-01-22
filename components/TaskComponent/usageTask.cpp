#include "usageTask.hpp"

using namespace Zotbins;

const gpio_num_t PIN_BREAKBEAM = GPIO_NUM_16;

static const char *name = "usageTask";
static const int priority = 1;
static const uint32_t stackSize = 4096;
static TaskHandle_t xTaskToNotify = NULL;
static const int core = 1;

// static void IRAM_ATTR breakbeam_isr(void *param)
// {
//     /* At this point xTaskToNotify should be NULL as no transmission
//        is in progress. A mutex can be used to guard access to the
//        peripheral if necessary. */
//     BaseType_t xHigherPriorityTaskWoken = pdFALSE;
//     configASSERT(xTaskToNotify == NULL);
//     /* Store the handle of the calling task. */
//     xTaskToNotify = xTaskGetCurrentTaskHandle();
//     vTaskNotifyGiveFromISR(xTaskToNotify, &xHigherPriorityTaskWoken);
// }

UsageTask::UsageTask(QueueHandle_t &messageQueue)
    : Task(name, priority, stackSize), mMessageQueue(messageQueue)
{
}

void UsageTask::start()
{
    xTaskCreatePinnedToCore(taskFunction, mName, mStackSize, this, mPriority, &xTaskToNotify, core); // create handle for task notification
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
    ESP_LOGI(name, "Hello from Usage Task");

    // Setup GPIO pin sensors
    gpio_set_direction(PIN_BREAKBEAM, GPIO_MODE_INPUT);

    while (1)
    {
        // Read in signal from breakbeam
        int detect = gpio_get_level(PIN_BREAKBEAM);

        // If breakbeam is disconnected
        if (detect == 1)
        {
            while (detect == 1)
            {

                ESP_LOGI(name, "Detecting item");
                detect = gpio_get_level(PIN_BREAKBEAM);
                vTaskDelay(1000 / portTICK_PERIOD_MS);
            }
            ESP_LOGI(name, "No longer detected");
            xTaskToNotify = xTaskGetHandle("fullnessTask");
            xTaskNotifyGive(xTaskToNotify);
            vTaskSuspend(NULL);
        }
        ESP_LOGI(name, "Nothing detected");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}