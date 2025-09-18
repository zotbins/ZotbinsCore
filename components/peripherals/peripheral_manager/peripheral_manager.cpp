#include <stdint.h>

#include "fullness_sensor.hpp"
#include "usage_sensor.hpp"
#include "weight_sensor.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "peripheral_manager.hpp"
#include "esp_log.h"

static const char* TAG = "peripheral_manager";

void init_manager(void) { // TODO: remove get_weight and get_distance calls, just init sensors here

    esp_err_t hx711_status = init_hx711();
    float weight = get_weight();

    esp_err_t hcsr04_status = init_hcsr04();
    float distance = get_distance();

    init_breakbeam(); // TODO: change return value to esp_err_t
    uint32_t usage = get_usage_count();

}