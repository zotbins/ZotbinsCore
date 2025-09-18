#ifndef USAGE_SENSOR_HPP
#define USAGE_SENSOR_HPP

#include "esp_err.h"

void init_breakbeam(void);
uint32_t get_usage_count(void);

#endif // USAGE_SENSOR_HPP