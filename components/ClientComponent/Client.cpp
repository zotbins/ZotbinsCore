/* MQTT over SSL Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include "esp_event.h"
#include "esp_netif.h"
#include "esp_partition.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "protocol_examples_common.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_tls.h"
#include "mqtt_client.h"
#include <sys/param.h>

#include "Client.hpp"
#include "Credentials.hpp"
#include "Serialize.hpp"
#include <cstdarg>
#include <cstring>

static const char *TAG = "mqtts_example";

static void publish(esp_mqtt_client_handle_t client, const void *data, size_t len)
{
    int msg_id = esp_mqtt_client_publish(client, "binData", (char *)data, len, 0, 0);
    ESP_LOGI(TAG, "message published with msg_id=%d", msg_id);
}

/*
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args user data registered to the event.
 * @param base Event base for the handler(always MQTT Base in this example).
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */
esp_mqtt_client_handle_t test_client;
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32, base, event_id);
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;
    esp_mqtt_client_handle_t client = event->client;
    test_client = client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        // msg_id = esp_mqtt_client_subscribe(client, "/topic/qos0", 0);
        // ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
        // publish(client, "Test");

        // msg_id = esp_mqtt_client_subscribe(client, "/topic/qos1", 1);
        // ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        // msg_id = esp_mqtt_client_unsubscribe(client, "/topic/qos1");
        // ESP_LOGI(TAG, "sent unsubscribe successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;
    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        // msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
        // publish(client, "Hello world!");
        ESP_LOGI(TAG, "sent publish successful");
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        ESP_LOGI(TAG, "TOPIC=%.*s\r\n", event->topic_len, event->topic);
        ESP_LOGI(TAG, "DATA=%.*s\r\n", event->data_len, event->data);
        // if (strncmp(event->data, "send binary please", event->data_len) == 0) {
        //     ESP_LOGI(TAG, "Sending the binary");
        //     publish(client, data);
        // }
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT)
        {
            ESP_LOGI(TAG, "Last error code reported from esp-tls: 0x%x", event->error_handle->esp_tls_last_esp_err);
            ESP_LOGI(TAG, "Last tls stack error number: 0x%x", event->error_handle->esp_tls_stack_err);
            ESP_LOGI(TAG, "Last captured errno : %d (%s)", event->error_handle->esp_transport_sock_errno,
                     strerror(event->error_handle->esp_transport_sock_errno));
        }
        else if (event->error_handle->error_type == MQTT_ERROR_TYPE_CONNECTION_REFUSED)
        {
            ESP_LOGI(TAG, "Connection refused error: 0x%x", event->error_handle->connect_return_code);
        }
        else
        {
            ESP_LOGW(TAG, "Unknown error type: 0x%x", event->error_handle->error_type);
        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

static void mqtt_app_start(void)
{
    const esp_mqtt_client_config_t mqtt_cfg = {
        .broker = {
            .address = {
                .uri = "mqtts://a1wqr7kl6hd1sm-ats.iot.us-west-1.amazonaws.com:8883",
            },
            .verification = {

                .certificate = (const char *)AWS_CA_CRT,
            },
        },
        .credentials = {.client_id = "SensorBin", .authentication = {
                                                      .certificate = (const char *)AWS_CLIENT_CRT,
                                                      .key = (const char *)AWS_CLIENT_KEY,
                                                  }}};

    ESP_LOGI(TAG, "Connecting to AWS server %s", AWS_URL);
    ESP_LOGI(TAG, "[APP] Free memory: %" PRIu32 " bytes", esp_get_free_heap_size());
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(client, MQTT_EVENT_ANY, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}

// TODO: optimize this into a dictionary or something
// #ifdef MCU_TYPE == CAMERA
    // bool payload;
// #elif MCU_TYPE == SENSOR
    bool payload_distance = false;
    bool payload_weight = false;
    float distance;
    int32_t weight;
// #endif 

// TODO: change temp to include the actual value through variadics
void Client::clientPublish(char* data_type, void* value)
{
    #if MCU_TYPE == CAMERA
        // cJSON* data = serialize(message, len);
    #elif MCU_TYPE == SENSOR
        if (strcmp(data_type, "distance") == 0){
            payload_distance = true;
            distance = *(float*)value;
        }else if (strcmp(data_type, "weight") == 0){
            payload_weight = true;
            weight = *(int32_t*)value;
        }
    #endif

    // only send when both distance and weight payloads are specified
    if (payload_distance && payload_weight){
        ESP_LOGI(TAG, "sending payload");
        ESP_LOGI(TAG, "distance = %d, weight = %d", payload_distance, payload_weight);
        cJSON* data = serialize("Sensor result", distance, false, weight);
        if (data) {
            char* json_str = cJSON_PrintUnformatted(data);  // Convert cJSON object to string
            if (json_str) {
                ESP_LOGI(TAG, "json string: %s", json_str);
                publish(test_client, json_str, strlen(json_str));  // Use strlen to get the size
                free(json_str);  // Free the allocated string after publishing
            }
            cJSON_Delete(data);  // Free cJSON object
        }
        payload_distance = false;
        payload_weight = false;
    }
}


// void Client::clientPublishStr(const char *message)
// {
//     Client::clientPublish();
// }

void Client::clientStart()
{
    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %" PRIu32 " bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("esp-tls", ESP_LOG_VERBOSE);
    esp_log_level_set("mqtt_client", ESP_LOG_VERBOSE);
    esp_log_level_set("mqtt_example", ESP_LOG_VERBOSE);
    esp_log_level_set("transport_base", ESP_LOG_VERBOSE);
    esp_log_level_set("transport", ESP_LOG_VERBOSE);
    esp_log_level_set("outbox", ESP_LOG_VERBOSE);

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());

    mqtt_app_start();
}