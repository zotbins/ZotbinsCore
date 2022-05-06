#include "fullnessTask.hpp"
#include "FullnessMetric.hpp"
#include <Arduino.h>

using namespace Zotbins;

static const char *name = "fullnessTask";
static const int priority = 1;
static const uint32_t stackSize = 4096;

FullnessTask::FullnessTask()
    : Task(name, priority, stackSize)
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
    while (1)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay for 1000 milliseconds
        log_i("Hello from Fullness Task");
    }
}