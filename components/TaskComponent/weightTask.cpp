#include "weightTask.hpp"
#include "WeightMetric.hpp"
#include "esp_log.h"
#include "RealWeight.hpp"

using namespace Zotbins;

static const char *name = "weightTask";
static const int priority = 1;
static const uint32_t stackSize = 4096;

WeightTask::WeightTask(QueueHandle_t &messageQueue)
    : Task(name, priority, stackSize), mMessageQueue(messageQueue)
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
    Weight::RealWeight weight_source;
    Weight::WeightMetric wm(weight_source);

    int32_t normalization = 1004700;
    double kg_measurement;
    int32_t diff;
    int32_t data;

    while (1)
    {
        data = wm.getWeight();
        diff = abs(data - normalization);
        kg_measurement = static_cast<double>(diff) / 19680.0;

        vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay for 1000 milliseconds
        ESP_LOGI(name, "Hello from Weight Task : %.2f kg", kg_measurement);
    }
}