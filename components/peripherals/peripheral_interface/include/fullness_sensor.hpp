#ifndef FULLNESS_SENSOR_HPP
#define FULLNESS_SENSOR_HPP

#include "esp_err.h"

/**
 * @brief Initializes the HC-SR04 ultrasonic sensor object and configure its pins.
 *
 * @return esp_err_t
 */
esp_err_t init_hcsr04(void);

/**
 * @brief Queries the HC-SR04 ultrasonic sensor for distance in centimeters.
 *
 * @return float
 */
float get_fullness(void);

#endif // FULLNESS_SENSOR_HPP