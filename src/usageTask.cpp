#include "usageTask.hpp"
#include <Arduino.h>

using namespace Zotbins;

static const char *name = "usageTask";
static const int priority = 1;
static const uint32_t stackSize = 4096;

UsageTask::UsageTask()
    : Task(name, priority, stackSize)
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
}

void UsageTask::loop()
{
    while (1)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay for 1000 milliseconds
        log_i("Hello from Usage Task");
    }
}