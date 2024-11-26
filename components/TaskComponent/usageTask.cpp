#include "usageTask.hpp"
#include "esp_log.h"

using namespace Zotbins;

static const char *name = "usageTask";
static const int priority = 1;
static const uint32_t stackSize = 4096;
static const SemaphorHandle_t = usageSem;


UsageTask::UsageTask(QueueHandle_t &messageQueue)
    : Task(name, priority, stackSize), mMessageQueue(messageQueue)
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
    usageSem = xSemaphorCreateBinary();
    
    
}

void UsageTask::loop()
{
    while (1)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay for 1000 milliseconds
        ESP_LOGI(name, "Hello from Usage Task");
    }
}