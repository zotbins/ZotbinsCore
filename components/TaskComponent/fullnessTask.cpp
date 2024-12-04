#include <driver/gpio.h>
#include "fullnessTask.hpp"
#include "FullnessMetric.hpp"
#include "DistanceBuffer.hpp"
#include "esp_log.h"

#define BIN_HEIGHT 1000

using namespace Zotbins;

const gpio_num_t PIN_TRIGGER = GPIO_NUM_12;
const gpio_num_t PIN_ECHO = GPIO_NUM_13;

static const char *name = "fullnessTask";
static const int priority = 1;
static const uint32_t stackSize = 4096;

FullnessTask::FullnessTask(QueueHandle_t &messageQueue)
    : Task(name, priority, stackSize), mMessageQueue(messageQueue)
{
}

void FullnessTask::start()
{
    xTaskCreate(taskFunction, mName, mStackSize, this, mPriority, nullptr);
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
        vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay for 1000 milliseconds
        distance = ultrasonic.getDistance();

        ESP_LOGI(name, "Hello from Fullness Task %f", distance);
        // ESP_LOGI(name, "Hello from Fullness Task");
        // gpio_set_level(PIN_TRIGGER, 0);
        // gpio_set_level(PIN_ECHO, 0);
        // vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay for 1000 milliseconds

    }
}