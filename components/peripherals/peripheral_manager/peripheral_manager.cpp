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

#include "mcp_dev.h"
#include "mcp_gpio_macros.h"

static const char *TAG = "peripheral_manager"; // Tag for ESP logging

// Define the global MCP device pointer (declared as extern in mcp_dev.h)
mcp23x17_t *mcp_dev = nullptr;
static TaskHandle_t manager_handle = nullptr; // Task handle for the peripheral manager task

EventGroupHandle_t manager_eg = nullptr; // Event group to signal when sensors have finished collecting data.

const uint8_t DEVICE_ADDR = 0x20;

void init_manager(void)
{

    // Initialize i2cdev subsystem (creates port mutexes and internal state)
    i2cdev_init();

    // Initialize empty devie, will break without
    static mcp23x17_t mcp23017_device = {};

    // I2C address, requires A0, A1, A2 tied to ground on device
    uint8_t mcp23017_addr = DEVICE_ADDR;

    /* External pullups preferred since internal pullups are 3.3v, 5v required for 1MHz on I2C. Therefore, keep the pullups disabled. 10kohm pullups work */

    // mcp23017_device.cfg.sda_pullup_en = 1; // enable internal SDA pull-up
    // mcp23017_device.cfg.scl_pullup_en = 1; // enable internal SCL pull-up

    // Initialize I2C line (SDA=12, SCL=14) (SDA is first)
    esp_err_t err = mcp23x17_init_desc(&mcp23017_device, mcp23017_addr, I2C_NUM_0, GPIO_NUM_12, GPIO_NUM_14);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "mcp23x17_init_desc failed: %s", esp_err_to_name(err));
        return;
    }

    mcp_dev = &mcp23017_device; // Set global MCP device pointer

    /* Read instructions for gpio expander. TODO */
    /* GPIO expander testing code */

    // uint16_t value;

    // while (1)
    // {
    //     vTaskDelay(100 / portTICK_PERIOD_MS);
    //     err = mcp23x17_port_read(&mcp23017_device, &value);
    //     if (err == ESP_OK)
    //     {
    //         ESP_LOGI(TAG, "port value=0x%04x", value);
    //     }
    //     else
    //     {
    //         ESP_LOGE(TAG, "mcp23x17_port_read failed: %s", esp_err_to_name(err));
    //     }
    // }

    // Initialize sensors

    // commented out; not implmeneted yet
    // esp_err_t hx711_status = init_hx711();

    init_hcsr04();
    init_breakbeam(); // TODO: change return value to esp_err_t

    manager_eg = xEventGroupCreate(); // Create the event group to store sensor event bits---for example, when the breakbeam is tripped, or when the servo has finished moving.

    // if (init_servo() == ESP_OK)
    // {
    //     servo_set_angle(0);
    // }

    xTaskCreate(
        run_manager,          /* Task function. */
        "peripheral_manager", /* name of task. */
        4096,                 /* Stack size of task */
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

    // temp
    while (1)
    {
        float fullness = get_fullness();
        uint32_t usage = get_usage_count();

        // read the breakbeam pin directly
        bool breakbeam_state;
        mcp23x17_get_level(mcp_dev, MCP_PORTB_GPIO1, &breakbeam_state);

        ESP_LOGI(TAG, "Fullness: %.2f, Usage: %d, Breakbeam: %s",
                 fullness, usage, breakbeam_state ? "HIGH" : "LOW");

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    // temp
    while (1)
    {
        float fullness = get_fullness();
        uint32_t usage = get_usage_count();
        ESP_LOGI(TAG, "Fullness: %.2f, Usage: %d", fullness, usage);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    while (1)
    {
        xEventGroupWaitBits(manager_eg, BIT0, pdTRUE, pdTRUE, portMAX_DELAY); // Wait for the breakbeam to be tripped, then collect sensor data.

        // Collect sensor data---add additional sensors here as needed
        // float weight = get_weight();
        float weight = 0; // temp
        float fullness = get_fullness();
        uint32_t usage = get_usage_count();

        // if (usage != last_usage)
        // { // Instructs servo to open bin
        //     servo_set_angle(90);
        //     gate_open = true;
        //     open_since = xTaskGetTickCount();
        //     last_usage = usage;
        // }

        // if (gate_open && (xTaskGetTickCount() - open_since) >= pdMS_TO_TICKS(kHoldMs))
        // { // Instructs servo to close bin
        //     servo_set_angle(0);
        //     gate_open = false;
        // }

        // Publish data
        publish_payload(fullness, weight, usage);
    }
}

static void publish_payload(float fullness, float weight, int usage)
{                                                       // TODO: allow variable number of sensor data parameters
    char *payload = serialize(fullness, weight, usage); // Serialize data as JSON string
    client_publish(payload);                            // Publish data to MQTT broker
}
