# ZotBins Communication Documentation 
This document includes all of the relevant information when it comes to the different device-device or device-server communication systems.

In the future this will be update with how we handle connectivity and some hardware we use to boost communication abilities. 

## AWS MQTT
### Server Credentials
In order for a client (ESP32 microcontroller) to interact with the server, it needs the correct credentials. The four main credentials are the following:

- **aws.url** - Contains the MQTT broker endpoint address
- **ca.crt (Certificate Authority Certificate)** - The root CA certificate used to verify the authenticity of the AWS server, ensuring you are connecting to a legitimate AWS endpoint
- **client.crt (Client Certificate)** - your device's unique public certificate that identifies and authenticates your specific device to the AWS server
- **Client.key (Client Private Key)** -the private key corresponding to your client certificate. That proves your device actually owns the client certificate and enables encrypted communication.

**Note:** In general, none of these credentials should be shared. They are a part of the.gitignore in ZotBinsCore. If you do not have the credentials or need a new policy message on the embedded channel, ping @chad on discord. Once you have the four files, you should paste them into ZotbinsCore\components\mqtt_client\credentials.

<img src="https://github.com/zotbins/ZotbinsCore/tree/main/docs/images/creds.png" alt="credentials">

**AWS Policies**
Each set of credentials is paired with a specific AWS policy. The AWS policy is a JSON document that defines what actions (permission) a client is allowed to perform when connected to the server. For example, device 1 and device 2 are able to connect and read from the server, but only device 1 is allowed to write to the server.

There are four main actions for the AWS IoT Core:

- **iot:Connect** - Allows a device to establish to an MQTT connection to the AWS IoT broker 
- **iot:Publish** - Allows a device to send (publish) messages to specific MQTT topics
- **iot:Subscribe** -  Allows a device to register in (subscribe) to specific MQTT topics
- **iot:Receive** - Allows a device to actually receive/download message from topics it has subscribed to.

An example AWS policy (this is not the actual ZotBin policy):

```
{
  "Version": "2012-10-17",
  "Statement": [
    {
      "Effect": "Allow",
      "Action": "iot:Connect",
      "Resource": "arn:aws:iot:us-west-2:123456789012:client/myDevice"
    },
    {
      "Effect": "Allow",
      "Action": "iot:Publish",
      "Resource": "arn:aws:iot:us-west-2:123456789012:topic/sensor/data"
    }
  ]
}
```

**Note:** There are multiple types of policies and corresponding credentials for ZotBins. If you are experiencing issues with the server, it is possible that is.

### Using the ZotBins MQTT Server 
There is client code on the micro controller, but it is also important to have access to the server. You can login to the server at this link: [AWS Management Console](https://aws.amazon.com/console/). You will be prompted with the following:

<img src="https://github.com/zotbins/ZotbinsCore/tree/main/docs/images/awsLogin.png" alt="AWS Login">

**Note:** If you need the login credentials message @chad in the embedded channel on discord

Once you are logged into the AWS server navigate to IoT Core:

<img src="https://github.com/zotbins/ZotbinsCore/tree/main/docs/images/iotCore.png" alt="IoTCore">

If you want to access the main console navigate to MQTT Test Client on the sidebar:

<img src="https://github.com/zotbins/ZotbinsCore/tree/main/docs/images/mqttClient.png" alt="MQTT Test Client">


Now you have the main console open. It should look somewhat like the following:

<img src="https://github.com/zotbins/ZotbinsCore/tree/main/docs/images/mqttConsole.png" alt="MQTT Console">


Topics are hierarchical strings that act as message routing channels in AWS IoT Core's MQTT-based pub/sub messaging system. The topic we use for most operations is called "binData." This is where all the data from the ESP32 will be published.

You can subscribe to this topic by typing in "binData" or "#" (wildcard, catches everything) to the topic filter and clicking the subscribe button. If a message is published to the topic it looks like this:

<img src="https://github.com/zotbins/ZotbinsCore/tree/main/docs/images/mqttSubscribe.png" alt="MQTT Subscribe">

By subscribing to the topic, you can look at all the different messages that are received by the server. If you want to publish directly to a topic from the server (perhaps to test if the server can send a message to the client), you can swap to the at the top to "publish to a topic."

<img src="https://github.com/zotbins/ZotbinsCore/tree/main/docs/images/mqttPublish.png" alt="MQTT Publish">

That is the main operation of API protocol from the side of the server. Here is some important information about the location.

For initialization and basic client connection, refer to the following script. This script has not changed in a while.
- ZotbinsCore\components\mqtt_client\client_connect.cpp

**client_connect.cpp code (Updated 11/24/2025):**

```c++
#include <stdio.h>
#include <stdint.h> // also for uint32_t type compability in header––must go before credentials inclusion
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "esp_system.h"
#include "esp_mac.h" // ESP-IDF Hint: esp_mac.h header file is not included by esp_system.h anymore. It shall then be manually included with #include "esp_mac.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_log.h"
#include "mqtt_client.h"

#include "credentials.hpp"
#include "client_connect.hpp"
#include "initialization.hpp"

static const char *TAG = "client_connect";
static esp_mqtt_client_handle_t client = nullptr;

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0)
    {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

extern EventGroupHandle_t sys_init_eg;

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
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;
    // change from void* to esp_mqtt_event_handle_t with event_data -> (esp_mqtt_event_handle_t)event_data
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        xEventGroupSetBits(sys_init_eg, CLIENT_CONNECT_STATUS_EVENT_BIT); // Bit_0 indicates MQTT connection established, sys_init_eg defined in initialization.cpp
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;
    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
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
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT)
        {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno", event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

void client_connect(void)
{
    // mqtts (secure) MAKE SURE TO INCLUDE THE CREDENTIALS IN THE FOLDER: mqtt_client/credentials/
    const esp_mqtt_client_config_t mqtt_cfg = {
        .broker = {
            .address = {
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

    ESP_LOGI(TAG, "URI=%s", mqtt_cfg.broker.address.uri); // Debug print to check URI

    client = esp_mqtt_client_init(&mqtt_cfg);
    ESP_LOGI(TAG, "Received mqtt client handle: %p", client);

    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(client, MQTT_EVENT_ANY, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}

void client_disconnect(void)
{
    esp_mqtt_client_stop(client);
    esp_mqtt_client_destroy(client);
    client = nullptr;
}

esp_mqtt_client_handle_t get_client_handle(void)
{
    return client;
}
```

For publishing to the MQTT server refer to the following script:
- ZotbinsCore\components\publish\client_publish.cpp

**client_publish.cpp code (Updated 11/24/2025):**
```c++
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
```

We send any important data in a json string format to the server (the data variable points to this). We use a serialization script that expedites the process of turning the raw sensor data into the correct json format. All messages will hit the endpoint, but in order for the data to be properly propagated to the rest of the API, it needs to be in the correct format (matching json fields, data types, etc.). Contact API leads if you are unsure about what they are expecting to receive for sensor data, camera data, or whatever else.

For example the serialization of sensor data looks like the following:
```c++
#include "serialize.hpp"

char *serialize(float fullness, float weight, int usage) // Copied from oldzotbinscore
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "bin_id", "newzotbin"); // Attach bin ID
    cJSON_AddNumberToObject(root, "fullness", fullness);  // Attach fullness (in centimeters) TODO: change to percentage
    cJSON_AddNumberToObject(root, "weight", weight);      // Attach weight (in grams) TODO: calibrate
    cJSON_AddNumberToObject(root, "usage", usage);        // Attach usage (number of trash items)

    char *payload = cJSON_PrintUnformatted(root); // Convert JSON object to string
    cJSON_Delete(root);                           // Free JSON object

    return payload;
}
```
We simplified most of the API systems after refactoring the entire codebase. Every other part of the pipeline, such as storing in the database and batch training, is handled by waste-rec/API. As of Fall 2025, we do not have real-time waste recognition fully designed yet.

**Important Technical Limitations of MQTT**
- MQTT is a text-only transfer protocol. For example, you can't upload image files (you would have to encode the image in base64 and send it).
- The max size of each MQTT message is 128kb. If the image encoding is too large you must break it up into chunks

## HTTP Communication
HTTP communication is a common way for allowing communication between a client and a webserver. If the ESP32 wants to communicate with something that is not connected to the AWS server, this may be helpful. In some cases where we need immediate waste classification, we connect directly to a ZotBins waste-rec server which is in DBH 2059.

There are different HTTP methods/operations:
- **GET** - Retrieve data from the server
- **POST** - Send data to server (such as sending sensor data)
- **PUT** - Update existing data
- **DELETE** - Remove Data

When an HTTP request occurs it will return a status code. The most common ones are listed below:
- **200** - OK (success)
- **201** - Created
- **400** - Bad Request
- **404** - Not Found 
- **500** - Internal Server Error

The general structure of an HTTP Request (only the important parts):
- **Header** - metadata about the response/request (Content type, Authorization, etc.)
- **Body** - The actual data being sent/received

Also, in order to get HTTP to work, you must have a Wi-Fi connection and an enabled TCP/IP stack. The HTTP Content still needs to be cleaned up, but the main script can be found in ZotbinsCore\components\publish\client_talk.

client_talk.cpp code (Updated 11/24/2025) (subject to heavy change refer to the actual branch for most up-to-date version): 
```c++
esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id) {
        case HTTP_EVENT_ON_DATA:
            if (evt->data_len > 0) {
                http_response.append(reinterpret_cast<const char*>(evt->data), evt->data_len);
            }
            break;
        default:
            break;
    }
    return ESP_OK;
}

// Helper function to send JSON data
static void _send_json(const std::string& json_body) {
    esp_http_client_config_t config = {
            .url = SERVER_URL,
            .method = HTTP_METHOD_POST,
            .event_handler = _http_event_handler,
        };

    esp_http_client_handle_t client = esp_http_client_init(&config);

    // Set headers and body
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, json_body.c_str(), json_body.length());

    // Perform the request
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        ESP_LOGI(TAG, "POST Status = %d",
                esp_http_client_get_status_code(client));

        ESP_LOGI(TAG, "Response (%d bytes):\n%s",
            http_response.size(),
            http_response.c_str());
    } else {
        ESP_LOGE(TAG, "HTTP POST request failed: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
}

void client_send_text(const std::string& text){
    ESP_LOGI(TAG, "Sending text data to server!");

    // Build JSON body with the provided text parameter
    std::string json_body = R"({"image_str": ")" + text + R"("})";
    
    _send_json(json_body);
}

void client_send_sensor(int usage, float fullness, float weight){
    ESP_LOGI(TAG, "Sending sensor data to server!");

    // Build JSON body with the metrics
    std::ostringstream json_stream;
    json_stream << std::fixed << std::setprecision(2);
    json_stream << R"({"usage": )" << usage 
                << R"(, "fullness": )" << fullness 
                << R"(, "weight": )" << weight 
                << R"(})";
    
    _send_json(json_stream.str());
}

void client_send_image(camera_fb_t *fb){
    if (!fb) {
        ESP_LOGE(TAG, "Invalid frame buffer");
        return;
    }

    ESP_LOGI(TAG, "Processing image: %zu bytes, format: %d", fb->len, fb->format);

    // Check if format is JPEG
    if (fb->format != PIXFORMAT_JPEG) {
        ESP_LOGE(TAG, "Image format is not JPEG! Format: %d", fb->format);
        return;
    }

    ESP_LOGI(TAG, "Sending JPEG image to server (%zu bytes)...", fb->len);

    // HTTP configuration
    esp_http_client_config_t config = {
            .url = SERVER_URL,
            .method = HTTP_METHOD_POST,
            .event_handler = _http_event_handler,
            .timeout_ms = 10000,  // 10 second timeout for large images
        };

    esp_http_client_handle_t client = esp_http_client_init(&config);

    // Set headers - sending raw JPEG data
    esp_http_client_set_header(client, "Content-Type", "image/jpeg");
    esp_http_client_set_post_field(client, (const char*)fb->buf, fb->len);

    // Perform the request
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        ESP_LOGI(TAG, "POST Status = %d",
                esp_http_client_get_status_code(client));

        ESP_LOGI(TAG, "Response (%d bytes):\n%s",
            http_response.size(),
            http_response.c_str());
    } else {
        ESP_LOGE(TAG, "HTTP POST request failed: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
}
```

**Important Technical Limitations of HTTP:**
- Significant connection overhead and worse message efficiency (HTTP headers are much larger than MQTT headers)
- Bi-Drectional Communication. HTTP is generally unreliable for constant/streamed server-to-client updates
- Also, there are a lot of potential security issues since the data is sent as plain text.

## Bluetooth Communication
Bluetooth use is still under development. I implemented a very basic system that uses something called Bluetooth Low Energy (BLE) instead of standard Bluetooth like the ones used by your phone. You can check out the code on the Bluetooth branch.

BLE only supports small-sized communication like text. It can not support large image files. We were thinking of implementing a Pokémon Go system where you can connect to local ZotBins when you visit them (a lot of potential security issues with this though).

BLE will not come up on your Bluetooth device for your phone. In order to see it, you must install the Lightblue app on your phone. If you set it up correctly, you will see a connection called "ESP32" on your app.

## ESP-NOW Communication 
ESP-NOW is a communication protocol that is specific to ESP32 Microcontrollers. ESP-NOW is implemented on some branches, but it is archived because it has not been properly ported to the new ZotBinsCore system.

ESP-NOW is designed for fast, low-power, peer-to-peer communication between devices without needing a traditional Wi-Fi router or access point. The devices communicate directly using their MAC addresses, which makes it very low latency and extremely power efficient.

We will potentially invest in ESP-NOW when we have multiple deployed bins that need to quickly communicate with neighbors.