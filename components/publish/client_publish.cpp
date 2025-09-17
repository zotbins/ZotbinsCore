#include "esp_log.h"
#include "mqtt_client.h"

#include "client_connect.hpp"
#include "client_publish.hpp"

void client_publish(const char* data) {
    esp_mqtt_client_handle_t client = get_client_handle();
    // Check if client is connected
    if (client == NULL) {
        ESP_LOGW("client_publish", "Not connected to MQTT broker!");
        return;
    }

    int msg_id = esp_mqtt_client_publish(client, "binData", data, 0, 0, 0);
    switch (msg_id) {
        case -1:
            ESP_LOGW("client_publish", "Failed to publish message to broker!");
            break;
        case -2:
            ESP_LOGW("client_publish", "Failed to publish: MQTT outbox is full!");
            break;
        default:
            ESP_LOGI("client_publish", "Message published, msg_id=%d", msg_id);
            break;
    }
}