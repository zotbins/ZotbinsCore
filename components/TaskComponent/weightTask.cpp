#include "weightTask.hpp"
#include "WeightMetric.hpp"
#include "esp_log.h"
#include "RealWeight.hpp"
#include "hx711.h"
#include "driver/ledc.h"

using namespace Zotbins;

const gpio_num_t PIN_DOUT = GPIO_NUM_2;
const gpio_num_t PIN_PD_SCK = GPIO_NUM_14;

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
    // Weight::RealWeight weight_source; // TODO: refactor into weight component
    // Weight::WeightMetric wm(weight_source);

    int32_t weight;
    bool ready;

    hx711_gain_t gain_setting = HX711_GAIN_A_64;

    hx711_t wm = {
        .dout = PIN_DOUT,
        .pd_sck = PIN_PD_SCK,
        .gain = gain_setting
    };

    hx711_init(&wm);

    while (1)
    {
        hx711_is_ready(&wm, &ready);
        if (ready) hx711_read_data(&wm, &weight);
        else weight = -1;
        vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay for 1000 milliseconds
        ESP_LOGI(name, "Hello from Weight Task : %d", static_cast<int>(weight));
    }
}