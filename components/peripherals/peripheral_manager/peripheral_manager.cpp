#include <stdint.h>

#include "fullness_sensor.hpp"
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

    xTaskCreate(
        run_manager,   /* Task function. */
        "fullness_sensor_task", /* name of task. */
        4096,                   /* Stack size of task */
        NULL,                   /* parameter of the task */
        1,                      /* priority of the task */
        &manager_handle                   /* Task handle to keep track of created task */
    );

}

static void run_manager(void *arg) { // TODO: temp implementation to test sensors
    while (1) {
        float weight = get_weight();
        float distance = get_distance();
        uint32_t usage = get_usage_count();
        
        publish_payload_temp(distance, weight, usage);

        vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay for 1 second
    }
}

static void publish_payload_temp(float fullness, float weight, int usage) {
    char *payload = serialize(fullness, weight, usage);
    client_publish(payload);
    free(payload);
}