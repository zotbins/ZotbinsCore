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

static const char *TAG = "weight_sensor"; // Tag for ESP logging

static const float SCALE_FACTOR = 1.0; // Calibration factor---weight is divided by this value to scale the reading to a chosen unit. TODO: calibrate
static const float OFFSET = 0.0;       // Calibration offset---weight is adjusted by this value (weight - OFFSET) to calibrate the reading to zero when no weight is applied. TODO: calibrate

static hx711_t hx711 = {};

esp_err_t init_hx711(mcp23x17_t *mcp23017_device, uint8_t data_pin, uint8_t clock_pin)
{

    hx711.gain = HX711_GAIN_A_128;

    // Update the HX711 object with the provided pin numbers
    hx711.dev = mcp23017_device;
    hx711.dout = data_pin;
    hx711.pd_sck = clock_pin;

    ESP_LOGI(TAG, "Initializing weight sensor...");

    esp_err_t hx711_device_status = hx711_init(&hx711);
    if (hx711_device_status != ESP_OK)
    {
        ESP_LOGW(TAG, "... could not initialize weight sensor!");
        return hx711_device_status;
    }
    else
    {
        ESP_LOGI(TAG, "... weight sensor initialized!");
        return ESP_OK;
    }
}

float get_weight(void)
{
    int32_t weight;
    float calibrated_weight;
    hx711_read_average(&hx711, 10, &weight);
    calibrated_weight = (-1) * (weight / SCALE_FACTOR) - OFFSET; // Apply calibration. Multipled by -1 because the amplifier in the HX711 inverts the signal.
    ESP_LOGI(TAG, "Weight of trash: %f%%", calibrated_weight);
    return calibrated_weight;
}