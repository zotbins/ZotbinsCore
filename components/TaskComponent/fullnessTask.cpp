#include <driver/gpio.h>
#include "fullnessTask.hpp"
#include "FullnessMetric.hpp"
#include "DistanceBuffer.hpp"
#include "esp_log.h"

#define PIN_TRIGGER GPIO_NUM_2
#define PIN_ECHO GPIO_NUM_4

#define BIN_HEIGHT 1000

using namespace Zotbins;

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

void FullnessTask::setup()
{
    
}

void FullnessTask::loop()
{

    uint32_t bin_height = BIN_HEIGHT;

    Fullness::DistanceBuffer buffer; // initialize distance buffer (with IDistance interface for interfacing with ultrasonic)
    Fullness::FullnessMetric bin(bin_height, buffer); // init fullness metric containing . takes a reference of the buffer, so values can be added to the buffer and the fullness metric will have access to that

    buffer.setTriggerPin(PIN_TRIGGER); // set trigger pin
    buffer.setEchoPin(PIN_ECHO); // set echo pin

    // int distance_temp; // change to consistent type later on (uint32_t, needed to print for testing and format specifiers are weird)

    while (1)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay for 1000 milliseconds
        ESP_LOGI(name, "Hello from Fullness Task");

        // distance_temp = buffer.getDistance();
        // if(distance_temp > 0) {
        //     buffer.recordDistance(distance_temp);
        // }

        // // TODO: call get distance
        // // TODO: after x number of calls (or when about to enqueue to the mqtt message queue), run a calculation using fullness metric on the distance buffer to get a single fullness percentage
        // // TODO: clear the buffer, repeat 

        // ESP_LOGI(name, "Hello from Fullness Task, %d", distance_temp);

    }
}