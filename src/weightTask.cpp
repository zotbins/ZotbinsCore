#include "weightTask.hpp"
#include "WeightMetric.hpp"
#include <Arduino.h>

using namespace Zotbins;

static const char *name = "weightTask";
static const int priority = 1;
static const uint32_t stackSize = 4096;

WeightTask::WeightTask(QueueHandle_t &messageQueue)
    : Task(name, priority, stackSize), mMessageQueue(messageQueue), mSensor(Weight::RealWeight())
{
}

void WeightTask::start()
{
    xTaskCreate(taskFunction, mName, mStackSize, this, mPriority, nullptr);
}

void WeightTask::taskFunction(void *task)
{
    WeightTask *weightTask = static_cast<WeightTask *>(task);
    weightTask->setup();
    weightTask->loop();
}

void WeightTask::setup()
{
    log_i("Weight task setting up!");
    // do nothing... no setup needed
}

void WeightTask::loop()
{
    while (1)
    {
        int32_t reading = mSensor.getWeight();
        log_i("Weight = %dkg", reading);
        vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay for 1000 milliseconds

    }
}