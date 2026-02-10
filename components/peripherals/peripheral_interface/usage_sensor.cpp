/**
 * @file usage_sensor.cpp
 * @author Alex Ikeda (ikedaas@uci.edu)
 * @brief Contains functions for initializing and receiving interrupts from the breakbeam sensor.
 * @version 0.1
 * @date 2025-09-20
 *
 * @copyright Copyright (c) 2025
 *
 */

#include <stdint.h>
#include <driver/gpio.h>
#include <esp_idf_lib_helpers.h>
#include <esp_timer.h>
#include <ets_sys.h>

#include "usage_sensor.hpp"
#include "esp_log.h"

static const char *TAG = "usage_sensor";

static uint32_t usage_count = 0;

esp_err_t init_breakbeam(mcp23x17_t *dev, uint8_t breakbeam, gpio_num_t interrupt)
{

    static const gpio_config_t PIN_INTERRUPT_CONFIG = {
        .pin_bit_mask = (1ULL << interrupt),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE};

    // Init breakbeam on gpio expander
    mcp23x17_set_mode(dev, breakbeam, MCP23X17_GPIO_INPUT);
    mcp23x17_set_interrupt(dev, breakbeam, MCP23X17_INT_LOW_EDGE); // interrupt on falling edge
    mcp23x17_set_int_out_mode(dev, MCP23X17_OPEN_DRAIN);           // open-drain interrupt output

    return ESP_OK;
}

uint32_t get_usage_count(void)
{
    ESP_LOGI(TAG, "Items since startup: %" PRIu32, usage_count);
    return usage_count;
}
