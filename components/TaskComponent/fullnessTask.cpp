#include "fullnessTask.hpp"
#include "DistanceBuffer.hpp"
#include "FullnessMetric.hpp"
#include "esp_log.h"
#include <driver/gpio.h>

#define BIN_HEIGHT 1000

using namespace Zotbins;

const gpio_num_t PIN_TRIGGER = GPIO_NUM_12;
const gpio_num_t PIN_ECHO = GPIO_NUM_13;

const gpio_config_t PIN_TRIGGER_CONFIG = {
    .pin_bit_mask = 0x00001000,
    .mode = GPIO_MODE_OUTPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_ENABLE,
    .intr_type = GPIO_INTR_DISABLE};

const gpio_config_t PIN_ECHO_CONFIG = {
    .pin_bit_mask = 0x00002000,
    .mode = GPIO_MODE_INPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE};

static const char *name = "fullnessTask";
static const int priority = 1;
static const uint32_t stackSize = 4096;
static TaskHandle_t xTaskToNotify = NULL;

// static void IRAM_ATTR breakbeam_isr(void *param)
// {
//     /* At this point xTaskToNotify should be NULL as no transmission
//        is in progress. A mutex can be used to guard access to the
//        peripheral if necessary. */
//     configASSERT( xTaskToNotify == NULL );

//     /* Store the handle of the calling task. */
//     xTaskToNotify = xTaskGetCurrentTaskHandle();

//     //xSemaphoreGiveFromISR(usageSem, nullptr);
// }

FullnessTask::FullnessTask(QueueHandle_t &messageQueue)
    : Task(name, priority, stackSize), mMessageQueue(messageQueue)
{
}

void FullnessTask::start()
{
    xTaskCreatePinnedToCore(taskFunction, mName, mStackSize, this, mPriority, nullptr, 1);
}

void FullnessTask::taskFunction(void *task)
{
    FullnessTask *fullnessTask = static_cast<FullnessTask *>(task);
    fullnessTask->setup();
    fullnessTask->loop();
}

void FullnessTask::setup() // could refactor into setup later but there are a lot of issues with scope
{
}

void FullnessTask::loop()
{

    gpio_config(&PIN_TRIGGER_CONFIG);
    gpio_config(&PIN_ECHO_CONFIG);

    // TODO: use distance buffer to get averages and discard outliers
    // uint32_t bin_height = BIN_HEIGHT; // TODO: NEED TO OVERLOAD CONSTRUCTOR TO SUPPORT MAX_DISTANCE
    float distance;

    Fullness::Distance ultrasonic(PIN_TRIGGER, PIN_ECHO);
    // gpio_set_direction(PIN_TRIGGER, GPIO_MODE_OUTPUT);
    // gpio_set_direction(PIN_ECHO, GPIO_MODE_OUTPUT);

    while (1)
    {
        // gpio_set_level(PIN_TRIGGER, 1);
        // gpio_set_level(PIN_ECHO, 1);
        ulTaskNotifyTake(pdTRUE, (TickType_t)portMAX_DELAY);
        distance = ultrasonic.getDistance();
        ESP_LOGI(name, "Hello from Fullness Task %f", distance);
        vTaskDelay(3000 / portTICK_PERIOD_MS);
        xTaskToNotify = xTaskGetHandle("usageTask");
        vTaskResume(xTaskToNotify);


        // ESP_LOGI(name, "Hello from Fullness Task");
        // gpio_set_level(PIN_TRIGGER, 0);
        // gpio_set_level(PIN_ECHO, 0);
        // vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay for 1000 milliseconds
    }
}