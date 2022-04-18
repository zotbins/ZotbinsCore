#include "usageTask.hpp"

using namespace Zotbins;

static const char *name = "usageTask";
static const int priority = 1;
static const uint32_t stackSize = 1024;

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
    }
}