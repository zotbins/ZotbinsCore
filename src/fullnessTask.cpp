#include "fullnessTask.hpp"
#include <Arduino.h>

using namespace Zotbins;

FullnessTask::FullnessTask()
    : Task(mName, mPriority, mStackSize)
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
    }
}