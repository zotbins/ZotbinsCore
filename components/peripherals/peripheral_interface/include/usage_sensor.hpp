#ifndef USAGE_SENSOR_HPP
#define USAGE_SENSOR_HPP

#include "esp_err.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "freertos/event_groups.h"
#include "mcp23x17.h"

extern EventGroupHandle_t manager_eg;

/**
 * @brief Configures breakbeam sensor pins and installs ISR handler.
 *
 */
esp_err_t init_breakbeam(mcp23x17_t *dev, uint8_t breakbeam, gpio_num_t interrupt);

/**
 * @brief Gets the total number of times the breakbeam has been triggered.
 *
 * @return uint32_t
 */
uint32_t get_usage_count(void);

#endif // USAGE_SENSOR_HPP