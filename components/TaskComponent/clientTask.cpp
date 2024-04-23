#include "Client.hpp"
#include "esp_log.h"


using namespace Zotbins;

static const char *name = "clientTask";
static const int priority = 1;
static const uint32_t stackSize = 4096;

ClientTask::ClientTask(QueueHandle_t &messageQueue)
    : Task(name, priority, stackSize), mMessageQueue(messageQueue)
{
}

void ClientTask::start()
{
    xTaskCreate(taskFunction, mName, mStackSize, this, mPriority, nullptr);
}

void ClientTask::taskFunction(void *task)
{
    ClientTask *clientTask = static_cast<ClientTask *>(task);
    clientTask->setup();
    clientTask->loop();
}

void ClientTask::setup()
{
}

void ClientTask::loop()
{
    while (1)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay for 1000 milliseconds
        ESP_LOGI(name, "Hello from Client Task");
    }
}