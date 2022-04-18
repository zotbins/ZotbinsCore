#include "usageTask.hpp"

using namespace Zotbins;

UsageTask::UsageTask()
    : Task(mName, mPriority, mStackSize)
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