#ifndef FULLNESS_SENSOR_HPP
#define FULLNESS_SENSOR_HPP

#include "esp_err.h"

esp_err_t init_hcsr04(void);
float get_distance(void);

#endif // FULLNESS_SENSOR_HPP