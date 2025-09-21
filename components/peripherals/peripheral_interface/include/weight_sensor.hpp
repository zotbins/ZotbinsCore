#ifndef WEIGHT_SENSOR_HPP
#define WEIGHT_SENSOR_HPP

#include "esp_err.h"

/**
 * @brief Initializes the HX711 weight sensor object and configure its pins.
 *
 * @return esp_err_t
 */
esp_err_t init_hx711(void);

/**
 * @brief Queries the HX711 weight sensor for weight in grams. NEEDS TO BE CALIBRATED
 *
 * @return float
 */
float get_weight(void);

#endif // WEIGHT_SENSOR_HPP