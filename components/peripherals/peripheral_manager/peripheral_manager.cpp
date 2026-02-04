/**
 * @file peripheral_manager.cpp
 * @author Alex Ikeda (ikedaas@uci.edu)
 * @brief Manages peripheral synchronization and publishing.
 * @version 0.1
 * @date 2025-09-20
 *
 * @copyright Copyright (c) 2025
 *
 */

#include <stdint.h>

#include "fullness_sensor.hpp"
#include "servo.hpp"
#include "usage_sensor.hpp"
#include "weight_sensor.hpp"

#include "peripheral_manager.hpp"
#include "client_publish.hpp"
#include "esp_log.h"
#include "serialize.hpp"
#include "events.hpp"
#include "mcp23x17.h"

static const char *TAG = "peripheral_manager"; // Tag for ESP logging
static TaskHandle_t manager_handle = nullptr;  // Task handle for the peripheral manager task

EventGroupHandle_t manager_eg = nullptr; // Event group to signal when sensors have finished collecting data.

static mcp23x17_t mcp23017_device = {}; // MCP23017 device descriptor

const uint8_t MCP23X17_DEV_ADDR = 0x20; // address for all pins tied to ground

const uint8_t PIN_TRIGGER = 2;                // GPIO pin for HC-SR04 trigger
const uint8_t PIN_ECHO = 8;                   // GPIO pin for HC-SR04 echo
const uint8_t PIN_BREAKBEAM = 9;              // GPIO pin for breakbeam sensor
const gpio_num_t PIN_INTERRUPT = GPIO_NUM_15; // GPIO pin for breakbeam interrupt

void init_manager(void)
{

    // Initialize i2cdev subsystem (creates port mutexes and internal state) must only be initialized ONCE
    i2cdev_init();

    // Initialize I2C line (SDA=13, SCL=14) (SDA is first in parameters) - Up to date for WROVER-DEV_01_02
    esp_err_t err = mcp23x17_init_desc(&mcp23017_device, MCP23X17_DEV_ADDR, I2C_NUM_0, GPIO_NUM_13, GPIO_NUM_14);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "mcp23x17_init_desc failed: %s", esp_err_to_name(err));
        return;
    }

    // TODO: bypass I2C limitations with interrupt INTB

    // Initialize sensors
    esp_err_t hx711_status = init_hx711();
    esp_err_t hcsr04_status = init_hcsr04(&mcp23017_device, PIN_TRIGGER, PIN_ECHO);              // trigger pin 2, echo pin 8
    esp_err_t breakbeam_status = init_breakbeam(&mcp23017_device, PIN_BREAKBEAM, PIN_INTERRUPT); // breakbeam pin 1, interrupt on GPIO 15

    manager_eg = xEventGroupCreate(); // Create the event group to store sensor event bits---for example, when the breakbeam is tripped, or when the servo has finished moving.

    xTaskCreate(
        run_manager,          /* Task function. */
        "peripheral_manager", /* name of task. */
        2048,                 /* Stack size of task */
        NULL,                 /* parameter of the task */
        1,                    /* priority of the task */
        &manager_handle       /* Task handle to keep track of created task */
    );
}

static void run_manager(void *arg)
{

    ESP_LOGI(TAG, "Peripheral manager started!");

    // Check initial states, for debug and clearing interrupt
    uint32_t gpio_state;

    while (1)
    {
        ESP_LOGI(TAG, "Breakbeam is broken; attemping to clear interrupt");
        while (gpio_get_level(PIN_INTERRUPT) == 0)
        {
            vTaskDelay(pdMS_TO_TICKS(1000)); // wait for interrupt to clear
            mcp23x17_get_level(&mcp23017_device, PIN_BREAKBEAM, &gpio_state);
        }
        xEventGroupClearBits(manager_eg, BIT0);                                // Clear the interrupt state event bit
        xEventGroupWaitBits(manager_eg, BIT0, pdFALSE, pdTRUE, portMAX_DELAY); // Wait for the breakbeam to be tripped, then collect sensor data.

        // Collect sensor data---add additional sensors here as needed
        float weight = get_weight();
        float fullness = get_fullness();
        uint32_t usage = get_usage_count();

        // Publish data
        publish_payload(fullness, weight, usage);
    }
}

static void publish_payload(float fullness, float weight, int usage)
{                                                       // TODO: allow variable number of sensor data parameters
    char *payload = serialize(fullness, weight, usage); // Serialize data as JSON string
    client_publish(payload);                            // Publish data to MQTT broker
}
