#include "esp_log.h"
#include "cJSON.h"

#include "freertos/FreeRTOS.h"
#include <freertos/task.h>

#include "weight_sensor.hpp"
#include "sensor_queue.hpp"

// bin_data_queue = xQueueCreate();
// QueueHandle_t g_mqtt_pub_q = xQueueCreate(/*len*/ 16, sizeof(mqtt_pub_t));

static const char* TAG = "sensor_queue";

void init_queue(void) {
    esp_err_t hx711_status = init_hx711();
    float weight = get_weight();
    ESP_LOGI(TAG, "Weight: %f", weight);
}