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

static const char *TAG = "peripheral_manager"; // Tag for ESP logging
static TaskHandle_t manager_handle = nullptr;  // Task handle for the peripheral manager task

EventGroupHandle_t manager_eg = nullptr; // Event group to signal when sensors have finished collecting data.

void init_manager(void)
{

    // Initialize sensors
    esp_err_t hx711_status = init_hx711();
    esp_err_t hcsr04_status = init_hcsr04();
    init_breakbeam(); // TODO: change return value to esp_err_t

    manager_eg = xEventGroupCreate(); // Create the event group to store sensor event bits---for example, when the breakbeam is tripped, or when the servo has finished moving.

    if (init_servo() == ESP_OK)
    {
        servo_set_angle(0);
    }

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

    // Servo parameters
    uint32_t last_usage = get_usage_count();
    bool gate_open = false;
    TickType_t open_since = 0;
    constexpr TickType_t kHoldMs = 600;

    while (1)
    {
        xEventGroupWaitBits(manager_eg, BIT0, pdTRUE, pdTRUE, portMAX_DELAY); // Wait for the breakbeam to be tripped, then collect sensor data.

        // Collect sensor data---add additional sensors here as needed
        float weight = get_weight();
        float fullness = get_fullness();
        uint32_t usage = get_usage_count();

        if (usage != last_usage)
        { // Instructs servo to open bin
            servo_set_angle(90);
            gate_open = true;
            open_since = xTaskGetTickCount();
            last_usage = usage;
        }

        if (gate_open && (xTaskGetTickCount() - open_since) >= pdMS_TO_TICKS(kHoldMs))
        { // Instructs servo to close bin
            servo_set_angle(0);
            gate_open = false;
        }

        // Publish data
        publish_payload(fullness, weight, usage);
    }
}

static void publish_payload(float fullness, float weight, int usage)
{                                                       // TODO: allow variable number of sensor data parameters
    char *payload = serialize(fullness, weight, usage); // Serialize data as JSON string
    client_publish(payload);                            // Publish data to MQTT broker
}
