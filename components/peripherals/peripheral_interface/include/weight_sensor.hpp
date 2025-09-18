#ifndef WEIGHT_SENSOR_HPP
#define WEIGHT_SENSOR_HPP

#include "esp_err.h"

esp_err_t init_hx711(void);
float get_weight(void);

#endif // WEIGHT_SENSOR_HPP