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

#include "esp_wifi.h"
// #include "esp_event.h"
// #include "esp_log.h"
// #include "esp_netif.h"


static const char *name = "mqtts_example";

static void publish(esp_mqtt_client_handle_t client, const void *data, size_t len)
{
    #if defined(SENSOR)
        int msg_id = esp_mqtt_client_publish(client, "binData", (char *)data, len, 0, 0);
    #elif defined(CAMERA)
        //int msg_id = esp_mqtt_client_publish(client, "photoData", (char *)data, len, 1, 0);
        printf("%zu\n", len); 

        int msg_id = esp_mqtt_client_publish(client, "photoData", (char *)data, len, 0, 0);
    #endif
    //int msg_id = esp_mqtt_client_publish(client, "photoData", (char *)data, len, 0, 0);
    //ESP_LOGI(name, "message published with msg_id=%d", msg_id);
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
bool client_connected = false;
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(name, "Event dispatched from event loop base=%s, event_id=%" PRIi32, base, event_id);
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;
    esp_mqtt_client_handle_t client = event->client;
    test_client = client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(name, "MQTT_EVENT_CONNECTED");
        client_connected = true;
        // msg_id = esp_mqtt_client_subscribe(client, "/topic/qos0", 0);
        // ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
        // publish(client, "Test", 5);

        // msg_id = esp_mqtt_client_subscribe(client, "/topic/qos1", 1);
        // ESP_LOGI(name, "sent subscribe successful, msg_id=%d", msg_id);

        // msg_id = esp_mqtt_client_unsubscribe(client, "/topic/qos1");
        // ESP_LOGI(name, "sent unsubscribe successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(name, "MQTT_EVENT_DISCONNECTED");
        break;
    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(name, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        // msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
        // publish(client, "Hello world!");
        ESP_LOGI(name, "sent publish successful");
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(name, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(name, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(name, "MQTT_EVENT_DATA");
        ESP_LOGI(name, "TOPIC=%.*s\r\n", event->topic_len, event->topic);
        ESP_LOGI(name, "DATA=%.*s\r\n", event->data_len, event->data);

        // // TODO: add that conditions determining TOPIC as either "OTA" or "PING"
        // if (strncmp(event->topic, "PING", event->topic_len) == 0) {
        //     // do ping to other ESP stuff
        //     ESP_LOGI(name, "Ping info received: %s", event->data);
        // }else if (strncmp(event->topic, "OTA", event->topic_len) == 0) {
        //     // do ota stuff
        //     // not implemented yet so make a policy under AWS certs for allowing pub/connect/receive
        // }
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(name, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT)
        {
            ESP_LOGI(name, "Last error code reported from esp-tls: 0x%x", event->error_handle->esp_tls_last_esp_err);
            ESP_LOGI(name, "Last tls stack error number: 0x%x", event->error_handle->esp_tls_stack_err);
            ESP_LOGI(name, "Last captured errno : %d (%s)", event->error_handle->esp_transport_sock_errno,
                     strerror(event->error_handle->esp_transport_sock_errno));
        }
        else if (event->error_handle->error_type == MQTT_ERROR_TYPE_CONNECTION_REFUSED)
        {
            ESP_LOGI(name, "Connection refused error: 0x%x", event->error_handle->connect_return_code);
        }
        else
        {
            ESP_LOGW(name, "Unknown error type: 0x%x", event->error_handle->error_type);
        }
        break;
    default:
        ESP_LOGI(name, "Other event id:%d", event->event_id);
        break;
    }
}

static void mqtt_app_start(void)
{
    // mqtts
    const esp_mqtt_client_config_t mqtt_cfg = {
        .broker = {
            .address = {
                // TODO: make sure address isn't hardcoded and go back to config files
                .uri = (const char *)AWS_URL,
            },
            .verification = {

                .certificate = (const char *)AWS_CA_CRT,
            },
        },
        .credentials = {.client_id = "SensorBin", .authentication = {
                                                      .certificate = (const char *)AWS_CLIENT_CRT,
                                                      .key = (const char *)AWS_CLIENT_KEY,
                                                  }}};

    // mqtt
    // const esp_mqtt_client_config_t mqtt_cfg = {
    //     .broker = {
    //         .address = {
    //             // TODO: make sure address isn't hardcoded and go back to config files
    //             .uri = (const char *)AWS_URL,
    //         }}};

    ESP_LOGI(name, "Connecting to AWS server %s", AWS_URL);
    ESP_LOGI(name, "[APP] Free memory: %" PRIu32 " bytes", esp_get_free_heap_size());
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    ESP_LOGI(name, "Initialized client.");

    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(client, MQTT_EVENT_ANY, mqtt_event_handler, NULL);
    bool yield = esp_mqtt_client_start(client);

    // // we don't need to worry too much on QoS so set at lvl 0
    // // if we really need packet info connectivity set as lvl 1 or 2 
    // // https://www.hivemq.com/blog/mqtt-essentials-part-6-mqtt-quality-of-service-levels/
    // while (!client_connected){
    //     vTaskDelay(1000 / portTICK_PERIOD_MS);
    //     ESP_LOGI(name, "yield for client connection");
    // }
    // ESP_LOGI(name, "connecting to test subscribe");
    // esp_mqtt_client_subscribe_single(client, "PING", 0); 
    // ESP_LOGI(name, "success subscribed");

    // // send example data (like payload stuff)
    // ESP_LOGI(name, "sending ping stuff");
    // esp_mqtt_client_publish(client, "PING", "esp32 stuff", 12, 0, 0);
}

// TODO: optimize this into a dictionary or something
// okay I did this but I reverted it back, I would just suggest only putting
// payload as a key and bool as a value and then making sure that gets checked to true 
// for publishing. The values in each are too specific` and concatenation from void is a pain,
// so you don't need to optimize that part
bool payload_camera = false;
char* imageData;
int uncompressedSize = 0;
int compressedSize = 0;
bool payload_distance = false;
bool payload_weight = false;
bool payload_usage = false;
float distance = 0;
float weight = 0;
int usage = 0;

// TODO: change temp to include the actual value through variadics
void Client::clientPublish(char* data_type, void* value)
{
    ESP_LOGI(name, "Publishing to broker.");
    cJSON* data = NULL;
    #if defined(CAMERA)
        if(strcmp(data_type, "camera") == 0){
            payload_camera = true;
            imageData = static_cast<char*>(value); 
            ESP_LOGE(name, "Camera Image Data Received");
        }
        // Potentially move the compression to Client task or somethign
        // else if (strcmp(data_type, "cameraCompressed") == 0){
        //     compressedSize = *static_cast<int*>(value);
        //     ESP_LOGE(name, "Camera Compressed Size Received");
        // }
        // else if (strcmp(data_type, "cameraUncompressed") == 0){ // Necessary for sending compressed length 
        //     uncompressedSize = *static_cast<int*>(value);
        //     ESP_LOGE(name, "Camera Uncompressed Size Received");
        // }
        if(payload_camera){
            ESP_LOGI(name, "sending camera payload");
            data = serialize("Camera result", imageData, strlen(imageData)); // Change compressed and uncosmpresed size from 1,1
        }
    #elif defined(SENSOR)
        if (strcmp(data_type, "usage") == 0){
            payload_usage = true;
            usage = *(int*)value;
        }else if (strcmp(data_type, "distance") == 0){
            payload_distance = true;
            distance = *(float*)value; 
        }else if (strcmp(data_type, "weight") == 0){
            payload_weight = true;
            weight = *(float*)value;
        }

        // only send when both distance and weight payloads are specified
        ESP_LOGI(name, "payload check use = %d, dis = %d, wei = %d", payload_usage, payload_distance, payload_weight);
        if (payload_distance) { // && payload_usage && payload_weight){
            ESP_LOGI(name, "sending payload");
            ESP_LOGI(name, "uses = %i, distance = %f, weight = %f", usage, distance, weight);
            data = serialize("Sensor result", distance, false, weight, usage);            
        }
    #endif

    // cleanup
    if (data != NULL) {
        char* json_str = cJSON_PrintUnformatted(data);  // Convert cJSON object to string
        if (json_str) {
            // TODO: ADD A DEBUG MODE THAT ENABLES THIS, BECAUSE THIS
            // ESP_LOGI(name, "json string: %s", json_str);
            publish(test_client, json_str, strlen(json_str));  // Use strlen to get the size
            free(json_str);  // Free the allocated string after publishing
        }
        cJSON_Delete(data);  // Free cJSON object
        #if defined(CAMERA)
            payload_camera = false;
            imageData = NULL;
        #elif defined(SENSOR)
            payload_usage = false;
            payload_distance = false;
            payload_weight = false;
            distance = 0;
            weight = 0;
        #endif
    }
}


void Client::clientPublishStr(const char *message)
{
    ESP_LOGI(name, "message: %s", message);
    publish(test_client, message, strlen(message));  // Use strlen to get the size
}

#define ESPNOW_MAXDELAY 512
#define CONFIG_ESPNOW_CHANNEL 1
#define ESPNOW_WIFI_MODE WIFI_MODE_STA
#define ESPNOW_WIFI_IF WIFI_IF_STA

void Client::clientStart()
{
    ESP_LOGI(name, "[APP] Startup..");
    ESP_LOGI(name, "[APP] Free memory: %" PRIu32 " bytes", esp_get_free_heap_size());
    ESP_LOGI(name, "[APP] IDF version: %s", esp_get_idf_version());

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

    // printf("fsdjfosdjfosdjfosd");
    // wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    // ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    // ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    // ESP_ERROR_CHECK(esp_wifi_set_channel(, WIFI_SECOND_CHAN_NONE));  // both on channel 1
    // ESP_ERROR_CHECK(esp_wifi_start());
    
        
    

    // TODO: when you disconnect make sure w reconnect socket if hostname fails
    /*
        E (2957998) esp-tls: couldn't get hostname for :a1wqr7kl6hd1sm-ats.iot.us-west-1.amazonaws.com: getaddrinfo() returns 202, addrinfo=0x0
        E (2957998) esp-tls: Failed to open new connection
        E (2957998) transport_base: Failed to open a new connection
        E (2958008) mqtt_client: Error transport connect
    */
    mqtt_app_start();

    // for measuring wifi speeds
    // while(1){
    //     vTaskDelay(1000  / portTICK_PERIOD_MS);
    //     wifi_ap_record_t ap_info;
    //     esp_err_t ret = esp_wifi_sta_get_ap_info(&ap_info);
        
    //     if (ret == ESP_OK) {
    //         int rssi = ap_info.rssi;
    //         printf("RSSI: %d dBm\n", rssi);
    //     } else {
    //         printf("Failed to get AP info\n");
    //     }
    // }
    
}