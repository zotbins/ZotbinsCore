#include "weightTask.hpp"
#include "driver/ledc.h"
#include "esp_log.h"
#include "hx711.h"
#include "nvs_flash.h" // for storing calibration data
#include "Client.hpp"

using namespace Zotbins;

const gpio_num_t PIN_DOUT = GPIO_NUM_14; // shifted since pcb pins are shifted. it should be 2 and 14 for dout and sck respectively
const gpio_num_t PIN_PD_SCK = GPIO_NUM_15; // shifted for same  reason. 15 used by servo, cant use servo on pin 15 simultaneously.
static TaskHandle_t xTaskToNotify = NULL;

const gpio_config_t PIN_PD_SCK_CONFIG = {
    .pin_bit_mask = 0x00008000,
    .mode = GPIO_MODE_OUTPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_ENABLE,
    .intr_type = GPIO_INTR_DISABLE
};

const gpio_config_t PIN_DOUT_CONFIG = {
    .pin_bit_mask = 0x00004000,
    .mode = GPIO_MODE_INPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE
};

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

    ESP_LOGI(name, "Waiting for hx711...");
    while(!ready) {
        hx711_is_ready(&wm, &ready); // checks if dout is low
    }
    hx711_read_average(&wm, 10, &tare_factor); // tare the scale during initialization when sensor is ready. // already waits for ready but added in for demonstration purposes (above while loop can be removed)
    ESP_LOGI(name, "Tared scale.");
    calibration_factor = 10000; // callibrate scale to lbs, empirically determined
    /* end of calibration */

    // ALTERNATIVE TO TARE ON INITIALIZATION: STORE TARE VALUE IN FLASH SO IT DOESN'T RESET ON STARTUP
    // // below was copied and modified from nvs_rw_value under storage in examples (esp-idf examples)
    // esp_err_t err = nvs_flash_init();
    // if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    // {
    //     // NVS partition was truncated and needs to be erased
    //     // Retry nvs_flash_init
    //     ESP_ERROR_CHECK(nvs_flash_erase());
    //     err = nvs_flash_init();
    // }
    // ESP_ERROR_CHECK(err);

    // // Open
    // printf("\n");
    // printf("Opening Non-Volatile Storage (NVS) handle... ");
    // nvs_handle_t my_handle;
    // err = nvs_open("storage", NVS_READWRITE, &my_handle);
    // if (err != ESP_OK)
    // {
    //     printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    // }
    // else
    // {
    //     printf("Done\n");

    //     // Read
    //     printf("Reading restart counter from NVS ... ");
    //     // int32_t restart_counter = 0; // value will default to 0, if not set yet in NVS
    //     err = nvs_get_i32(my_handle, "tare_factor", &tare_factor); // checking if the tare factor is already written to nvs
    //     switch (err)
    //     {
    //     case ESP_OK:
    //         printf("Done\n");
    //         printf("Tare factor = %" PRIu32 "\n", tare_factor);
    //         tare_factor_initialized = true;
    //         break;
    //     case ESP_ERR_NVS_NOT_FOUND:
    //         printf("The value is not initialized yet!\n");
    //         tare_factor_initialized = false;
    //         break;
    //     default:
    //         printf("Error (%s) reading!\n", esp_err_to_name(err));
    //         tare_factor_initialized = true; // skip initializing, something wrong with accessing the tare_factor from nvs
    //     }

    //     // TODO: fix calibration factor since it's causing scale to not work properly
    //     err = nvs_get_i32(my_handle, "calibration_factor", &calibration_factor); // checking if the calibration factor is already written to nvs
    //     switch (err)
    //     {
    //     case ESP_OK:
    //         printf("Done\n");
    //         printf("Tare factor = %" PRIu32 "\n", calibration_factor);
    //         calibration_factor_initialized = true;
    //         break;
    //     case ESP_ERR_NVS_NOT_FOUND:
    //         printf("The value is not initialized yet!\n");
    //         calibration_factor_initialized = false;
    //         break;
    //     default:
    //         printf("Error (%s) reading!\n", esp_err_to_name(err));
    //         calibration_factor_initialized = true; // skip initializing, something wrong with accessing the tare_factor from nvs
    //     }

    //     if (tare_factor_initialized == false) { // if the tare_factor has not already been set, measured and set it
    //         ESP_LOGI(name, "Waiting for hx711...");
    //         while(!ready) {
    //             hx711_is_ready(&wm, &ready); // checks if dout is low
    //         }
    //         hx711_read_average(&wm, 10, &tare_factor); // tare the scale during initialization when sensor is ready. // already waits for ready but added in for demonstration purposes (above while loop can be removed)
    //         // get the raw weight when there is nothing on the sensor, so this reading can be considered zero weight (tare).

    //         err = nvs_set_i32(my_handle, "tare_factor", tare_factor); // write
    //         printf((err != ESP_OK) ? "tare_factor writing failed!\n" : "Done\n");
    //     }

    //     if (calibration_factor_initialized == false)
    //     { // if the tare_factor has not already been set, measured and set it
    //         /* TODO: calibrate - idea is to proceed with this after the tare factor is initialized, and
    //             then wait for a significant change in weight, or until some sort of input
    //             (might need a separate pin for this or pause all the other tasks to avoid pin conflicts)
    //             and then get the calibration factor from this. or we can meauser it and write it directly
    //             into nvs using a constant in program memory but that is messier
    //         */
    //         calibration_factor = 1;

    //         err = nvs_set_i32(my_handle, "calibration_factor", calibration_factor); // write
    //         printf((err != ESP_OK) ? "calibration_factor writing failed!\n" : "Done\n");
    //     }

    //     // Commit wriing any values, nvs_commit() must be called to ensure changes are written
    //     // to flash storage. Implementations may write to storage at other times,
    //     // but this is not guaranteed.
    //     printf("Committten value. After settting updates in NVS ... ");
    //     err = nvs_commit(my_handle);
    //     printf((err != ESP_OK) ? "Failed!\n" : "Done\n");

    //     // Close
    //     nvs_close(my_handle);
    // }

    while (1)
    {
        ESP_LOGI(name, "Starting weight loop...");
        ulTaskNotifyTake(pdTRUE, (TickType_t)portMAX_DELAY);

        ESP_ERROR_CHECK(gpio_set_level(wm.pd_sck, 0)); // reset clock pulse
        ready = false; // reset hx711 ready status

        while(!ready) {
            ESP_LOGI(name, "Waiting for hx711...");
            hx711_is_ready(&wm, &ready); // checks if dout is low
        }
        ESP_LOGI(name, "hx711 is ready.");

        if (ready) // if dout is low, ready = !dout, read the weight
        {
            ESP_LOGI(name, "Getting weight...");
            hx711_read_average(&wm, 10, &weight_raw); 
            ESP_LOGI(name, "Raw weight: %ld", weight_raw);

            weight = tare_factor + (-1) * (weight_raw);
            // weight_raw is inverted; therefore, we need to invert the measurement (this is what the -1 is for). then we add this reading to the tare factor which zeroes out the scale when nothing in placed on the sensor.
            weight = weight / calibration_factor;
            // calibration factor is an int that scales up or down the weight reading from an arbitraty number to one in any other unit. it is divided by the calibration factor so it can be an int, since most often the reading will be scaled downwards and nvs_flash only supports portable types like ints. (this should be done before deployment)
            if(weight < 0) weight = 0;
        }
        else
        {
            weight_raw = -1;
        }

        // Client::clientPublish("weight", static_cast<void*>(&weight));

        vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay for 1000 milliseconds
        ESP_LOGI(name, "Hello from Weight Task : %f", (weight));
        xTaskToNotify = xTaskGetHandle("usageTask"); // pass back to usage task  
        vTaskResume(xTaskToNotify);
    }
    vTaskDelete(NULL);
}
