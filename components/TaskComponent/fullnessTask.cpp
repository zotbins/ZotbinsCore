#include "fullnessTask.hpp"
#include "FullnessMetric.hpp"
#include "esp_log.h"

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
    while (1)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay for 1000 milliseconds
        ESP_LOGI(name, "Hello from Fullness Task");
    }
}