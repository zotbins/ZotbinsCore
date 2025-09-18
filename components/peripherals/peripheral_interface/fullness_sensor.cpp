#include <stdint.h>

#include "driver/gpio.h"
#include "ultrasonic.h"

#include "esp_log.h"
#include "fullness_sensor.hpp"

/*  
    GPIO Pin Configuation
    DO NOT declare pin numbers as static to avoid duplicate pin assignments.
    Please refer to the ESP32-WROVER datasheet for the pinouts.
    https://www.espressif.com/sites/default/files/documentation/esp32-wrover-e_esp32-wrover-ie_datasheet_en.pdf
*/ 
const gpio_num_t PIN_TRIG = GPIO_NUM_22; // TODO: reason for pin choice
const gpio_num_t PIN_ECHO = GPIO_NUM_23; // TODO: reason for pin choice

// NECESSARY for some pins. Always safer to use a gpio_config_t to configure pins.
static const gpio_config_t PIN_TRIG_CONFIG = {
    .pin_bit_mask = 1ULL << PIN_TRIG,
    .mode = GPIO_MODE_OUTPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_ENABLE,
    .intr_type = GPIO_INTR_DISABLE
};
static const gpio_config_t PIN_ECHO_CONFIG = {
    .pin_bit_mask = 1ULL << PIN_ECHO,
    .mode = GPIO_MODE_INPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE
};

static const char* TAG = "fullness_sensor";

static const int priority = 1;

static ultrasonic_sensor_t hcsr04 = {
    .trigger_pin = PIN_TRIG,
    .echo_pin = PIN_ECHO
};

// TODO: check sys_init_eg bits

esp_err_t init_hcsr04(void) {

    ESP_LOGI(TAG, "Initializing fullness sensor...");
    // ABSOLUTELY NECESSARY FOR SOME PINS, COMPLETELY OVERRIDES PREVIOUS CONFIGURATION
    ESP_ERROR_CHECK_WITHOUT_ABORT(
        gpio_config(&PIN_TRIG_CONFIG)
    );
    ESP_ERROR_CHECK_WITHOUT_ABORT(
        gpio_config(&PIN_ECHO_CONFIG)
    );

    esp_err_t hcsr04_device_status = ultrasonic_init(&hcsr04);
    if (hcsr04_device_status != ESP_OK) {
        ESP_LOGW(TAG, "Could not initialize fullness sensor...");
        return hcsr04_device_status;
    } else {
        return ESP_OK;
    }
}

float get_distance(void) {
    uint32_t distance;
    ultrasonic_measure_cm(&hcsr04, 1000, &distance); // TODO: convert to percentage
    ESP_LOGI(TAG, "Distance to trash: %"PRIu32" cm", distance);
    return distance;
}