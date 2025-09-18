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

    // client_connect();
    // client_publish("Hello from ESP32!");

    /* Event group initialization */
    sys_init_eg = xEventGroupCreate();
    ESP_LOGI(TAG, "System initialization event group created");

    ESP_LOGI(TAG, "Waiting for client to intialize...");
    xEventGroupWaitBits(sys_init_eg, BIT0, pdTRUE, pdTRUE, portMAX_DELAY); // Wait for MQTT connection to be established
    init_manager();

    vEventGroupDelete(sys_init_eg);
    ESP_LOGI(TAG, "System initialization event group deleted");

}
