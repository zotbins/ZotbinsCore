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

#define DEBOUNCE_TIME_MS 50
static TimerHandle_t debounce_timer = NULL;

static const char *TAG = "usage_sensor";

/*
    GPIO Pin Configuation
    DO NOT declare pin numbers as static to avoid duplicate pin assignments.
    Please refer to the ESP32-WROVER datasheet for the pinouts.
    https://www.espressif.com/sites/default/files/documentation/esp32-wrover-e_esp32-wrover-ie_datasheet_en.pdf
    As well as the ESP32 Technical Reference Manual for interrupt capabilities.
    https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/gpio.html
*/

/*
    ESP32: Please do not use the interrupt of GPIO36 and GPIO39 when using ADC or Wi-Fi and Bluetooth with sleep mode enabled. Please refer to the comments of adc1_get_raw. Please refer to Section 3.11 of ESP32 ECO and Workarounds for Bugs for the description of this issue.
*/

const gpio_num_t PIN_BREAKBEAM = GPIO_NUM_18;

// Intention with GPIO_INTR_NEGEDGE and the reduced ISR is to reduce the time the ISR is running, the only thing the ISR needs to do is mark rising and falling edge and increment or set values accordingly.
const gpio_config_t PIN_BREAKBEAM_CONFIG = {
    .pin_bit_mask = (1ULL << PIN_BREAKBEAM),
    .mode = GPIO_MODE_INPUT,
    .pull_up_en = GPIO_PULLUP_ENABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_NEGEDGE};

static uint32_t usage_count = 0;

static void debounce_timer_callback(TimerHandle_t xTimer) {
    gpio_intr_enable(PIN_BREAKBEAM);
}

// Breakbeam ISR
void IRAM_ATTR increment_usage(void *arg)
{
    
    BaseType_t xHigherPriorityTaskWoken, xResult; // from https://www.freertos.org/Documentation/02-Kernel/04-API-references/12-Event-groups-or-flags/06-xEventGroupSetBitsFromISR
    xHigherPriorityTaskWoken = pdFALSE; // Must be initialized to pdFALSE.

    gpio_intr_disable(PIN_BREAKBEAM);
    xTimerStartFromISR(debounce_timer, &xHigherPriorityTaskWoken);

    usage_count++; // Increment usage count

    xResult = xEventGroupSetBitsFromISR(manager_eg, BIT0, &xHigherPriorityTaskWoken); // Signal the manager task that the breakbeam was tripped
    
    if (xResult != pdFAIL)
    {
        // If unblocked task is higher priority than the daemon task, request an immediate context switch
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken); // Allows context switch wihtout waiting for the next tick.
    }
}

void init_breakbeam(void)
{
    debounce_timer = xTimerCreate(
        "debounce",
        pdMS_TO_TICKS(DEBOUNCE_TIME_MS),
        pdFALSE,  // One-shot timer
        NULL,
        debounce_timer_callback
    );

    ESP_LOGI(TAG, "Initializing usage sensor...");
    // ABSOLUTELY NECESSARY FOR SOME PINS, COMPLETELY OVERRIDES PREVIOUS CONFIGURATION
    ESP_ERROR_CHECK_WITHOUT_ABORT(
        gpio_config(&PIN_BREAKBEAM_CONFIG));

    ESP_LOGI(TAG, "Adding ISR handler...");
    ESP_ERROR_CHECK_WITHOUT_ABORT(
        gpio_isr_handler_add(PIN_BREAKBEAM, increment_usage, NULL));

    ESP_LOGI(TAG, "Usage sensor initialized!");
}

uint32_t get_usage_count(void)
{
    ESP_LOGI(TAG, "Items since startup: %" PRIu32, usage_count);
    return usage_count;
}
