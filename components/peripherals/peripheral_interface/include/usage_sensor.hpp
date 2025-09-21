#ifndef USAGE_SENSOR_HPP
#define USAGE_SENSOR_HPP

#include "esp_err.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "freertos/event_groups.h"

extern EventGroupHandle_t manager_eg;

void init_breakbeam(void);
uint32_t get_usage_count(void);

#endif // USAGE_SENSOR_HPP