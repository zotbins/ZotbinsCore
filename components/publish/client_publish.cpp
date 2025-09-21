/**
 * @file client_publish.cpp
 * @author Alex Ikeda (ikedaas@uci.edu)
 * @brief Functions for publishing data to the MQTT broker.
 * @version 0.1
 * @date 2025-09-20
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "esp_log.h"
#include "mqtt_client.h"

#include "client_connect.hpp"
#include "client_publish.hpp"

static const char *TAG = "client_publish"; // Tag for ESP logging

void client_publish(const char *data)
{
    esp_mqtt_client_handle_t client = get_client_handle();
    // Check if client is connected
    if (client == NULL)
    {
        ESP_LOGW(TAG, "Client is not connected to MQTT broker!");
        return;
    }

    int msg_id = esp_mqtt_client_publish(client, "binData", data, 0, 0, 0);
    switch (msg_id)
    {
    case -1:
        ESP_LOGW(TAG, "Failed to publish message to broker!");
        break;
    case -2:
        ESP_LOGW(TAG, "Failed to publish: MQTT outbox is full!");
        break;
    default:
        ESP_LOGI(TAG, "Message published, msg_id=%d", msg_id);
        break;
    }
}