#include "weightTask.hpp"

using namespace Zotbins;

static const char *name = "weightTask";
static const int priority = 1;
static const uint32_t stackSize = 1024;

WeightTask::WeightTask()
    : Task(name, priority, stackSize)
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
}

void WeightTask::loop()
{
    while (1)
    {
    }
}