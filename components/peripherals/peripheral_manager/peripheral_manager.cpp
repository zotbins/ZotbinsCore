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

#define GPA0 0x0001
#define GPA1 0x0002
#define GPA2 0x0004
#define GPA3 0x0008
#define GPA4 0x0010
#define GPA5 0x0020
#define GPA6 0x0040
#define GPA7 0x0080
#define GPB0 0x0100
#define GPB1 0x0200
#define GPB2 0x0400
#define GPB3 0x0800
#define GPB4 0x1000
#define GPB5 0x2000
#define GPB6 0x4000
#define GPB7 0x8000

static const char *TAG = "peripheral_manager"; // Tag for ESP logging
static TaskHandle_t manager_handle = nullptr;  // Task handle for the peripheral manager task

EventGroupHandle_t manager_eg = nullptr; // Event group to signal when sensors have finished collecting data.

const uint8_t DEVICE_ADDR = 0x20;

void init_manager(void)
{

    // Initialize i2cdev subsystem (creates port mutexes and internal state)
    i2cdev_init();

    // Initialize empty devie, will break without
    mcp23x17_t mcp23017_device = {};

    // I2C address, requires A0, A1, A2 tied to ground on device
    uint8_t mcp23017_addr = DEVICE_ADDR;

    /* External pullups preferred since internal pullups are 3.3v, 5v required for 1MHz on I2C. Therefore, keep the pullups disabled. 10kohm pullups work */

    // mcp23017_device.cfg.sda_pullup_en = 1; // enable internal SDA pull-up
    // mcp23017_device.cfg.scl_pullup_en = 1; // enable internal SCL pull-up

    // Initialize I2C line (SDA=12, SCL=14) (SDA is first)
    esp_err_t err = mcp23x17_init_desc(&mcp23017_device, mcp23017_addr, I2C_NUM_0, GPIO_NUM_12, GPIO_NUM_14);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "mcp23x17_init_desc failed: %s", esp_err_to_name(err));
        return;
    }

    uint16_t val;

    // 1 is input, 0 is output
    // Set pin (GPA0) to input (0x0001)
    err = mcp23x17_port_set_mode(&mcp23017_device, 0xFFFF & GPA0);
    if (err != ESP_OK) {
        val = mcp23x17_port_get_pullup(&mcp23017_device, &val);
        ESP_LOGW(TAG, "mcp23x17_port_set_mode returned %s", esp_err_to_name(err));
    } else {
        val = mcp23x17_port_get_pullup(&mcp23017_device, &val);
        ESP_LOGW(TAG, "mcp23x17_port_set_mode returned %" PRIu32, val);
    }

    // Set pullup resistors on pins
    err = mcp23x17_port_set_pullup(&mcp23017_device, 0xFFFF & GPA0);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "mcp23x17_port_set_pullup returned %s", esp_err_to_name(err));
    } else {
        val = mcp23x17_port_get_pullup(&mcp23017_device, &val);
        ESP_LOGW(TAG, "mcp23x17_port_set_pullup returned %" PRIu32, val);
    }

    uint16_t value;

    /* Read instructions for gpio expander. TODO */
    while (1) {
        vTaskDelay(100 / portTICK_PERIOD_MS);
        err = mcp23x17_port_read(&mcp23017_device, &value);
        if (err == ESP_OK) {
            ESP_LOGI(TAG, "port value=0x%04x", value);
        } else {
            ESP_LOGE(TAG, "mcp23x17_port_read failed: %s", esp_err_to_name(err));
        }
    }

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
