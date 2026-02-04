/**
 * @file fullness_sensor.cpp
 * @author Alex Ikeda (ikedaas@uci.edu)
 * @brief Contains functions for initializing and querying the HC-SR04 ultrasonic sensor.
 * @version 0.1
 * @date 2025-09-20
 *
 * @copyright Copyright (c) 2025
 *
 */

#include <stdint.h>

#include "driver/gpio.h"
#include "ultrasonic.h"

#include "esp_log.h"
#include "fullness_sensor.hpp"

static const char *TAG = "fullness_sensor"; // Tag for ESP logging

static const float MAX_DISTANCE = 400.0; // Maximum measurable distance in cm. Used in this formula: 100% - (distance / MAX_DISTANCE * 100%) = fullness percentage. TODO: calibrate

static ultrasonic_sensor_t hcsr04 = {}; // HC-SR04 device descriptor

esp_err_t init_hcsr04(mcp23x17_t *dev, uint8_t trigger, uint8_t echo)
{

    ESP_LOGI(TAG, "Initializing fullness sensor...");

    hcsr04 = {
        .dev = dev,
        .trigger_pin = trigger,
        .echo_pin = echo};

    esp_err_t hcsr04_device_status = ultrasonic_init(&hcsr04);

    if (hcsr04_device_status != ESP_OK)
    {
        ESP_LOGW(TAG, "Could not initialize fullness sensor...");
        return hcsr04_device_status;
    }
    else
    {
        ESP_LOGI(TAG, "Fullness sensor initialized!");
        return ESP_OK;
    }
}

float get_fullness(void)
{
    uint32_t distance;
    float fullness;
    ultrasonic_measure_cm(&hcsr04, MAX_DISTANCE, &distance); // TODO: convert to percentage
    fullness = 100.0 - (distance / MAX_DISTANCE * 100.0);    // Convert distance to fullness percentage

    if (fullness > 100.0 && fullness < 110.0)
    { // Likely a sensor measurement error, cap at 100%
        fullness = 100.0;
    }

    else if (fullness < 0.0)
    { // Floor at 0% --- THIS CAN CAUSE CONFUSION, MAKE SURE THE MAXIMUM DISTANCE IS CALIBRATED PROPERLY
        fullness = 0.0;
    }

    else if (fullness >= 110.0)
    { // Something is probably wrong, time to check up on the bin...
        ESP_LOGW(TAG, "Fullness reading out of range: %f%%", fullness);
        return fullness;
    }

    ESP_LOGI(TAG, "Distance to object: %" PRIu32 " cm", distance);
    ESP_LOGI(TAG, "Bin fullness: %f%%", fullness);
    return fullness;
}