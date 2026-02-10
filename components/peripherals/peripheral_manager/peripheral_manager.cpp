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
#include "serialize.hpp"
#include "events.hpp"
#include "mcp23x17.h"

#include "peripheral_manager.hpp"
#include "pin_assignments.hpp"

#include "client_publish.hpp"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

static const char *TAG = "peripheral_manager"; // Tag for ESP logging
static TaskHandle_t manager_handle = nullptr;  // Task handle for the peripheral manager task

/*
 
PERIPHERAL MANAGER EVENT GROUP NOTES

BIT0 - Interrupt event
BIT1 - Breakbeam state

Need assignment:
- Ultrasonic reading is pending
- Offline / online (?) may go in a different event group
- GPIO expander is initialized
- I2C is initialized
- Time sensitive task is currently being executed, other I2C transactions should be paused or queued
 
*/
EventGroupHandle_t manager_eg = nullptr; // Event group to signal when sensors have finished collecting data.

static mcp23x17_t mcp23017_device = {}; // MCP23017 device descriptor
const uint8_t MCP23X17_DEV_ADDR = 0x20; // address for all pins tied to ground
static volatile int64_t timestamp = 0; // Timestamp of the most recent interrupt, in microseconds. Used for debugging and potentially for deciding whether to queue interrupts.

void IRAM_ATTR mcp23017_isr_handler(void *arg) // Needs to timestamp every interrupt
{
    BaseType_t xHigherPriorityTaskWoken, xResult; // from https://www.freertos.org/Documentation/02-Kernel/04-API-references/12-Event-groups-or-flags/06-xEventGroupSetBitsFromISR

    xHigherPriorityTaskWoken = pdFALSE; // Must be initialized to pdFALSE.
    xResult = xEventGroupSetBitsFromISR(manager_eg, BIT0, &xHigherPriorityTaskWoken); // Signal the manager task that an interrupt has occured

    timestamp = esp_timer_get_time();

    // Add some control flow here; ex. if the interrupt was triggered while BIT0 was already set, then queue something. (LOW PRIOITY TODO)

    if (xResult != pdFAIL)
    {
        // If unblocked task is higher priority than the daemon task, request an immediate context switch
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken); // Allows context switch wihtout waiting for the next tick.
    }
}

void init_manager(void)
{
    // Initialize i2cdev subsystem (creates port mutexes and internal state) must only be initialized ONCE
    i2cdev_init();

    // Initialize mcp23017 device descriptor
    esp_err_t err = mcp23x17_init_desc(&mcp23017_device, MCP23X17_DEV_ADDR, I2C_NUM_0, PIN_SDA, PIN_SCL);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "mcp23x17_init_desc failed: %s", esp_err_to_name(err));
        return;
    }

    /* MCP23017 interrupt config */
    esp_err_t err;
    ESP_LOGI(TAG, "Initializing usage sensor interrupt...");
    err = gpio_config(&PIN_INTERRUPT_CONFIG);
    if (err != ESP_OK)
    {
        ESP_LOGW(TAG, "Failed to configure GPIO interrupt: %s", esp_err_to_name(err));
        return err;
    }
    ESP_LOGI(TAG, "Installing ISR service...");
    err = gpio_install_isr_service(0);
    if (err != ESP_OK)
    {
        ESP_LOGW(TAG, "Failed to install ISR service: %s", esp_err_to_name(err));
        return err;
    }
    ESP_LOGI(TAG, "Adding ISR handler...");
    err = gpio_isr_handler_add(PIN_INTERRUPT, mcp23017_isr_handler, NULL);
    if (err != ESP_OK)
    {
        ESP_LOGW(TAG, "Failed to add ISR handler: %s", esp_err_to_name(err));
        return err;
    }
    ESP_LOGI(TAG, "MCP23017 interrupt initialized!");
    /* END MCP23017 interrupt config */

    // Initialize sensors
    esp_err_t hx711_status = init_hx711(&mcp23017_device, PIN_DATA, PIN_CLOCK); // Needs additional parameters in definition. move pin defitions into manager
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
        xEventGroupWaitBits(manager_eg, BIT0, pdFALSE, pdTRUE, portMAX_DELAY); // Wait for the breakbeam to be tripped, then collect sensor data.
        /* Check interrupt flag register, then read capture register and extract value of pin that caused interrupt. Then decide if it was ultrasonic, etc. */

        // // Collect sensor data---add additional sensors here as needed
        // float weight = get_weight();
        // float fullness = get_fullness();
        // uint32_t usage = get_usage_count();

        // // Publish data
        // publish_payload(fullness, weight, usage);
    }
}

static void publish_payload(float fullness, float weight, int usage)
{                                                       // TODO: allow variable number of sensor data parameters
    char *payload = serialize(fullness, weight, usage); // Serialize data as JSON string
    client_publish(payload);                            // Publish data to MQTT broker
}