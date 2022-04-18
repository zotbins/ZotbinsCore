#include "weightTask.hpp"

using namespace Zotbins;

WeightTask::WeightTask()
    : Task(mName, mPriority, mStackSize)
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