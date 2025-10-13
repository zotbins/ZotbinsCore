#include "esp_log.h"
#include "cJSON.h"

#include "freertos/FreeRTOS.h"
#include <freertos/task.h>

#include "weight_sensor.hpp"
#include "fullness_sensor.hpp"
#include "peripheral_queue.hpp"

// bin_data_queue = xQueueCreate();
// QueueHandle_t g_mqtt_pub_q = xQueueCreate(/*len*/ 16, sizeof(mqtt_pub_t));

static const char *TAG = "peripheral_queue";

void init_queue(void)
{
}