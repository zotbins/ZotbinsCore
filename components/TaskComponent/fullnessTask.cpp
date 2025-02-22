#include "fullnessTask.hpp"
#include "DistanceBuffer.hpp"
#include "FullnessMetric.hpp"
#include "esp_log.h"
#include <driver/gpio.h>

using namespace Zotbins;

const gpio_num_t PIN_TRIGGER = GPIO_NUM_12;
const gpio_num_t PIN_ECHO = GPIO_NUM_13;
const float BIN_HEIGHT = 100;

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
static const int core = 1;

FullnessTask::FullnessTask(QueueHandle_t &messageQueue)
    : Task(name, priority, stackSize), mMessageQueue(messageQueue)
{
}

void FullnessTask::start()
{
    xTaskCreatePinnedToCore(taskFunction, mName, mStackSize, this, mPriority, &xTaskToNotify, core);
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

    ESP_ERROR_CHECK(gpio_config(&PIN_TRIGGER_CONFIG));
    ESP_ERROR_CHECK(gpio_config(&PIN_ECHO_CONFIG));

    // TODO: use distance buffer to get averages and discard outliers
    // uint32_t bin_height = BIN_HEIGHT; // TODO: NEED TO OVERLOAD CONSTRUCTOR TO SUPPORT MAX_DISTANCE
    float distance;
    float fullness;

    Fullness::Distance ultrasonic(PIN_TRIGGER, PIN_ECHO);
    // gpio_set_direction(PIN_TRIGGER, GPIO_MODE_OUTPUT);
    // gpio_set_direction(PIN_ECHO, GPIO_MODE_OUTPUT);

    while (1)
    {

        ulTaskNotifyTake(pdTRUE, (TickType_t)portMAX_DELAY);
        distance = ultrasonic.getDistance();
        if (distance < 0) {
            ESP_LOGI(name, "Error reading distance.");
        } else {
            fullness = distance / BIN_HEIGHT;
            ESP_LOGI(name, "Hello from Fullness Task: %f%%", distance);
        }
        xTaskToNotify = xTaskGetHandle("weightTask"); // get rtos handle for weight task
        xTaskNotifyGive(xTaskToNotify); // using that handle notify weight task to collect

    }
    vTaskDelete(NULL);
}
