#include <stdint.h>

#include "fullness_sensor.hpp"
#include "servo.hpp"
#include "usage_sensor.hpp"
#include "weight_sensor.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "peripheral_manager.hpp"
#include "client_publish.hpp"
#include "esp_log.h"
#include "serialize.hpp"

static const char* TAG = "peripheral_manager";
static TaskHandle_t manager_handle = NULL;

void init_manager(void) { // TODO: remove get_weight and get_distance calls, just init sensors here

    esp_err_t hx711_status = init_hx711();
    float weight = get_weight();

    esp_err_t hcsr04_status = init_hcsr04();
    float distance = get_distance();

    init_breakbeam(); // TODO: change return value to esp_err_t
    uint32_t usage = get_usage_count();

    if (init_servo() == ESP_OK) {
        servo_set_angle(0);
    }

    xTaskCreate(
        run_manager,   /* Task function. */
        "fullness_sensor_task", /* name of task. */
        4096,                   /* Stack size of task */
        NULL,                   /* parameter of the task */
        1,                      /* priority of the task */
        &manager_handle                   /* Task handle to keep track of created task */
    );

}

static void run_manager(void *arg) {
    uint32_t last_usage = get_usage_count();
    bool gate_open = false;
    TickType_t open_since = 0;
    constexpr TickType_t kHoldMs = 600;

    while (1) {
        float weight = get_weight();
        float distance = get_distance();
        uint32_t usage = get_usage_count();

        if (usage != last_usage) { // Open bin
            servo_set_angle(90);
            gate_open = true;
            open_since = xTaskGetTickCount();
            last_usage = usage;
        }

        if (gate_open && (xTaskGetTickCount() - open_since) >= pdMS_TO_TICKS(kHoldMs)) { // Close bin
            servo_set_angle(0);
            gate_open = false;
        }
        
        publish_payload_temp(distance, weight, usage);
        
        vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay for 1 second
    }
}

static void publish_payload_temp(float fullness, float weight, int usage) {
    char *payload = serialize(fullness, weight, usage);
    client_publish(payload);
    free(payload);
}