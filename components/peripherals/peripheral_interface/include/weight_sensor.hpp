#ifndef WEIGHT_SENSOR_HPP
#define WEIGHT_SENSOR_HPP

#include "esp_err.h"

/**
 * @brief Initializes the HX711 weight sensor object and configure its pins.
 *
 * @return esp_err_t
 */
esp_err_t init_hx711(mcp23x17_t *mcp23017_device, uint8_t data_pin, uint8_t clock_pin); // Needs additional parameters in definition. move pin defitions into manager

/**
 * @brief Queries the HX711 weight sensor for weight in grams. NEEDS TO BE CALIBRATED
 *
 * @return float
 */
float get_weight(void);

#endif // WEIGHT_SENSOR_HPP