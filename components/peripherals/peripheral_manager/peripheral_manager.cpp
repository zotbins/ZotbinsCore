#include <stdint.h>

#include "fullness_sensor.hpp"
#include "usage_sensor.hpp"
#include "weight_sensor.hpp"

#include "peripheral_manager.hpp"
#include "client_publish.hpp"
#include "esp_log.h"
#include "serialize.hpp"
#include "events.hpp"

static const char* TAG = "peripheral_manager";
static TaskHandle_t manager_handle = nullptr;

EventGroupHandle_t manager_eg = nullptr; // Event group to signal when sensors have finished collecting data

void init_manager(void) { // TODO: remove get_weight and get_distance calls, just init sensors here

    esp_err_t hx711_status = init_hx711();
    float weight = get_weight();

    esp_err_t hcsr04_status = init_hcsr04();
    float distance = get_distance();

    init_breakbeam(); // TODO: change return value to esp_err_t
    uint32_t usage = get_usage_count();

    manager_eg = xEventGroupCreate(); // Create the event group to signal when sensors have finished collecting data

    xTaskCreate(
        run_manager,   /* Task function. */
        "peripheral_manager", /* name of task. */
        2048,                   /* Stack size of task */
        NULL,                   /* parameter of the task */
        1,                      /* priority of the task */
        &manager_handle                   /* Task handle to keep track of created task */
    );

}

static void run_manager(void *arg) { // TODO: temp implementation to test sensors

    ESP_LOGI(TAG, "Peripheral manager started");

    while (1) {
        xEventGroupWaitBits(manager_eg, BIT0, pdTRUE, pdTRUE, portMAX_DELAY); // Wait for the breakbeam to be tripped, then collect sensor data.
        float weight = get_weight();
        float distance = get_distance();
        uint32_t usage = get_usage_count();
        
        publish_payload_temp(distance, weight, usage);
    }
}

static void publish_payload_temp(float fullness, float weight, int usage) {
    char *payload = serialize(fullness, weight, usage);
    client_publish(payload);
    free(payload);
}