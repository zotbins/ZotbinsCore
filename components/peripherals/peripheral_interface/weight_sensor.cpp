#include <stdint.h>

#include "driver/gpio.h"
#include "hx711.h"

#include "esp_log.h"
#include "weight_sensor.hpp"

/*  
    GPIO Pin Configuation
    DO NOT declare pin numbers as static to avoid duplicate pin assignments.
    Please refer to the ESP32-WROVER datasheet for the pinouts.
    https://www.espressif.com/sites/default/files/documentation/esp32-wrover-e_esp32-wrover-ie_datasheet_en.pdf
*/ 
const gpio_num_t PIN_DT = GPIO_NUM_2; // TODO: reason for pin choice
const gpio_num_t PIN_SCK = GPIO_NUM_14; // TODO: reason for pin choice

// NECESSARY for some pins. Always safer to use a gpio_config_t to configure pins.
static const gpio_config_t PIN_DT_CONFIG = {
    .pin_bit_mask = 1ULL << PIN_DT,
    .mode = GPIO_MODE_INPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_ENABLE,
    .intr_type = GPIO_INTR_DISABLE
};
static const gpio_config_t PIN_SCK_CONFIG = {
    .pin_bit_mask = 1ULL << PIN_SCK,
    .mode = GPIO_MODE_OUTPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE
};

static const char* TAG = "weight_sensor";

static const int priority = 1;

static hx711_t hx711 = {
    .dout = PIN_DT,
    .pd_sck = PIN_SCK,
    .gain = HX711_GAIN_A_128
};

// TODO: check sys_init_eg bits

esp_err_t init_hx711(void) {
    
    ESP_LOGI(TAG, "Initializing weight sensor...");
    // ABSOLUTELY NECESSARY FOR SOME PINS, COMPLETELY OVERRIDES PREVIOUS CONFIGURATION
    ESP_ERROR_CHECK_WITHOUT_ABORT(
        gpio_config(&PIN_DT_CONFIG)
    );
    ESP_ERROR_CHECK_WITHOUT_ABORT(
        gpio_config(&PIN_SCK_CONFIG)
    );

    esp_err_t hx711_device_status = hx711_init(&hx711);
    if (hx711_device_status != ESP_OK) {
        ESP_LOGW(TAG, "Could not initialize weight sensor...");
        return hx711_device_status;
    } else {
        return ESP_OK;
    }
}

float get_weight(void) {
    int32_t weight;
    hx711_read_average(&hx711, 10, &weight);
    ESP_LOGI(TAG, "Weight of trash: %" PRId32, weight);
    return weight;
}