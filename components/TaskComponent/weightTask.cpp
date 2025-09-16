#include "weightTask.hpp"
#include "Client.hpp"
#include "driver/ledc.h"
#include "esp_log.h"
#include "hx711.h"
#include "nvs_flash.h" // for storing calibration data
#include <math.h>      // for fabs
#include <set>

using namespace Zotbins;

const gpio_num_t PIN_DOUT = GPIO_NUM_2;
const gpio_num_t PIN_PD_SCK = GPIO_NUM_14;
static TaskHandle_t xTaskToNotify = NULL;

const gpio_config_t PIN_DOUT_CONFIG = {
    .pin_bit_mask = 1ULL << PIN_DOUT,
    .mode = GPIO_MODE_INPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_ENABLE,
    .intr_type = GPIO_INTR_DISABLE};

const gpio_config_t PIN_PD_SCK_CONFIG = {
    .pin_bit_mask = 1ULL << PIN_PD_SCK,
    .mode = GPIO_MODE_OUTPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE};

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

    ESP_ERROR_CHECK(gpio_config(&PIN_DOUT_CONFIG)); // ensure pins is configured as gpio, especially necessary for pins 12-15 and just in case for other pins
    ESP_ERROR_CHECK(gpio_config(&PIN_PD_SCK_CONFIG));

    int32_t weight;
    bool ready;

    hx711_gain_t gain_setting = HX711_GAIN_A_128;

    hx711_t wm = {
        .dout = PIN_DOUT,
        .pd_sck = PIN_PD_SCK,
        .gain = gain_setting
    };

    hx711_init(&wm);

    while (1)
    {
        ulTaskNotifyTake(pdTRUE, (TickType_t)portMAX_DELAY);
        gpio_set_level(wm.pd_sck, 0);

        hx711_is_ready(&wm, &ready);
        hx711_read_average(&wm, 10, &weight);

        Client::clientPublish("weight", static_cast<void *>(&weight));

        vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay for 1000 milliseconds
        ESP_LOGI(name, "Hello from Weight Task : %d", static_cast<int>(weight));

        // ESP_LOGI(name, "Hello from Weight Task : %f", (weight));
        xTaskToNotify = xTaskGetHandle("usageTask");
        vTaskResume(xTaskToNotify);
    }
}