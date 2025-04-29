#include "weightTask.hpp"
#include "driver/ledc.h"
#include "esp_log.h"
#include "hx711.h"
#include "nvs_flash.h" // for storing calibration data
#include "Client.hpp"

using namespace Zotbins;

const gpio_num_t PIN_DOUT = GPIO_NUM_14; // shifted since pcb pins are swapped when you plug directly in
const gpio_num_t PIN_PD_SCK = GPIO_NUM_15; // shifted for same reason. 15 used by servo, cant use servo on pin 15 simultaneously.
static TaskHandle_t xTaskToNotify = NULL;

const gpio_config_t PIN_PD_SCK_CONFIG = {
    .pin_bit_mask = 0x00008000,
    .mode = GPIO_MODE_OUTPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_ENABLE,
    .intr_type = GPIO_INTR_DISABLE};

const gpio_config_t PIN_DOUT_CONFIG = {
    .pin_bit_mask = 0x00004000,
    .mode = GPIO_MODE_INPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE};

static const char *name = "weightTask";
static const int priority = 1;
static const uint32_t stackSize = 4096;
static const int core = 1;

WeightTask::WeightTask(QueueHandle_t &messageQueue)
    : Task(name, priority, stackSize), mMessageQueue(messageQueue)
{
}

void WeightTask::start()
{
    xTaskCreatePinnedToCore(taskFunction, mName, mStackSize, this, mPriority, &xTaskToNotify, core);
}

void WeightTask::taskFunction(void *task)
{
    WeightTask *weightTask = static_cast<WeightTask *>(task);
    weightTask->setup();
    weightTask->loop();
}

void WeightTask::setup()
{
    // TODO: move all necessary setup variables to members of a weighttask object
}

float WeightTask::getWeight(){
    return weight;
}

void WeightTask::loop()
{
    ESP_ERROR_CHECK(gpio_config(&PIN_DOUT_CONFIG)); // ensure pins is configured as gpio, especially necessary for pins 12-15 and just in case for other pins
    ESP_ERROR_CHECK(gpio_config(&PIN_PD_SCK_CONFIG));

    int32_t weight_raw;
    int32_t tare_factor;
    int32_t calibration_factor;
    bool ready; // variable storing the status of the weight sensor measurement (measurement ready to be read or not)
    bool tare_factor_initialized;
    bool calibration_factor_initialized; // factor to scale raw weight reading to grams

    /* sensor initialization */
    hx711_gain_t gain_setting = HX711_GAIN_A_128;

    hx711_t wm = {// construct weight object that specifies the pins to use and the gain of the hx711 amplifier
                  .dout = PIN_DOUT,
                  .pd_sck = PIN_PD_SCK,
                  .gain = gain_setting};

    hx711_init(&wm);
    /* end of initialization */

    /* calibration */
    ESP_ERROR_CHECK(gpio_set_level(wm.pd_sck, 0));
    ready = false;

    while(!ready) {
        ESP_LOGI(name, "Waiting for hx711...");
        hx711_is_ready(&wm, &ready); // checks if dout is low
    }
    hx711_read_average(&wm, 10, &tare_factor); // tare the scale during initialization when sensor is ready
    calibration_factor = 10000; // callibrate scale to lbs, empirically determined
    /* end of calibration */

    while (1)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        ESP_ERROR_CHECK(gpio_set_level(wm.pd_sck, 0));
        ready = false;

        int retry = 0;
        while (!ready && retry < 10) {
            hx711_is_ready(&wm, &ready);
            if (!ready) {
                ESP_LOGW("WeightTask", "HX711 not ready yet... retrying (%d)", retry);
                vTaskDelay(100 / portTICK_PERIOD_MS);
                retry++;
            }
        }

        if (!ready) {
            ESP_LOGE("WeightTask", "HX711 never became ready!");
            continue;
        }

        try {
            esp_err_t err = hx711_read_average(&wm, 10, &weight_raw);
            if (err != ESP_OK) {
                ESP_LOGE("WeightTask", "Error reading from HX711: %s", esp_err_to_name(err));
                continue;
            }

            weight = tare_factor + (-1) * weight_raw;
            weight = weight / calibration_factor;

            ESP_LOGI("WeightTask", "Final weight: %.2f", weight);
            Client::clientPublish("weight", static_cast<void*>(&weight));

        } catch (const std::exception &e) {
            ESP_LOGE("WeightTask", "Caught exception: %s", e.what());
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);
        xTaskToNotify = xTaskGetHandle("usageTask");
        if (xTaskToNotify != nullptr) {
            vTaskResume(xTaskToNotify);
        } else {
            ESP_LOGW("WeightTask", "usageTask handle not found!");
        }
    }
    vTaskDelete(NULL);
}
