/**
 * @file weight_sensor.cpp
 * @author Alex Ikeda (ikedaas@uci.edu)
 * @brief Contains functions for initializing and querying the HX711 weight sensor.
 * @version 0.1
 * @date 2025-09-20
 *
 * @copyright Copyright (c) 2025
 *
 */

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
const gpio_num_t PIN_DT = GPIO_NUM_2;   // TODO: reason for pin choice
const gpio_num_t PIN_SCK = GPIO_NUM_14; // TODO: reason for pin choice

// NECESSARY for some pins. Always safer to use a gpio_config_t to configure pins.
static const gpio_config_t PIN_DT_CONFIG = {
    .pin_bit_mask = 1ULL << PIN_DT,
    .mode = GPIO_MODE_INPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_ENABLE,
    .intr_type = GPIO_INTR_DISABLE};
static const gpio_config_t PIN_SCK_CONFIG = {
    .pin_bit_mask = 1ULL << PIN_SCK,
    .mode = GPIO_MODE_OUTPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE};

static const char *TAG = "weight_sensor"; // Tag for ESP logging

static const float SCALE_FACTOR = 1.0; // Calibration factor---weight is divided by this value to scale the reading to a chosen unit. TODO: calibrate
static const float OFFSET = 13700.0;       // Calibration offset---weight is adjusted by this value (weight - OFFSET) to calibrate the reading to zero when no weight is applied. TODO: calibrate

static hx711_t hx711 = { // HX711 sensor object
    .dout = PIN_DT,
    .pd_sck = PIN_SCK,
    .gain = HX711_GAIN_A_128};

esp_err_t init_hx711(void)
{

    ESP_LOGI(TAG, "Initializing weight sensor...");

    // ABSOLUTELY NECESSARY FOR SOME PINS, COMPLETELY OVERRIDES PREVIOUS CONFIGURATION
    ESP_ERROR_CHECK_WITHOUT_ABORT(
        gpio_config(&PIN_DT_CONFIG));
    ESP_ERROR_CHECK_WITHOUT_ABORT(
        gpio_config(&PIN_SCK_CONFIG));

    esp_err_t hx711_device_status = hx711_init(&hx711);
    if (hx711_device_status != ESP_OK)
    {
        ESP_LOGW(TAG, "Could not initialize weight sensor...");
        return hx711_device_status;
    }
    else
    {
        ESP_LOGI(TAG, "Weight sensor initialized!");
        return ESP_OK;
    }
}

float get_weight(void)
{
    int32_t weight;
    float calibrated_weight;
    hx711_read_average(&hx711, 10, &weight);
    ESP_LOGI(TAG, "Raw weight reading: %" PRId32, weight);
    calibrated_weight = (weight / SCALE_FACTOR) - OFFSET; // Apply calibration. Multipled by -1 because the amplifier in the HX711 inverts the signal.
    ESP_LOGI(TAG, "Weight of trash: %f", calibrated_weight);
    return calibrated_weight;
}