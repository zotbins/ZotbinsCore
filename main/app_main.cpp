/* MQTT (over TCP) Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"

#include "esp_log.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "client_connect.hpp"
#include "client_publish.hpp"

#include "peripheral_manager.hpp"

#include "freertos/event_groups.h"
#include "esp_system.h"
#include "initialization.hpp"

static const char *TAG = "app_main";

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %" PRIu32 " bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    /* Connecting to WiFi */
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    // TODO: REPLACE WITH MANUAL CONNECTION TO AVOID DEPENDENCY
    ESP_ERROR_CHECK(example_connect());
    ESP_LOGI(TAG, "Connected to AP");

    /* Event group initialization */

    extern EventGroupHandle_t sys_init_eg; // sys_init_eg defined in initialization.cpp
    initialize(); // create the event group, from initialization.cpp -- TODO: add other initialization conditions. This one waits for the MQTT connection to be established before initializing the sensors.

    // MQTT client initialization and connection
    client_connect();
    client_publish("Hello from ESP32!");

    // Wait for MQTT connection to be established
    ESP_LOGI(TAG, "Waiting for client to intialize...");
    xEventGroupWaitBits(sys_init_eg, BIT0, pdTRUE, pdTRUE, portMAX_DELAY);
    ESP_LOGI(TAG, "Client initialized");

    init_manager(); // Initialize peripheral manager after system initialization is complete, this manages the sensors (peripheral_manager.cpp)

    vEventGroupDelete(sys_init_eg); // Free the event group
    ESP_LOGI(TAG, "System initialization event group deleted");

}
