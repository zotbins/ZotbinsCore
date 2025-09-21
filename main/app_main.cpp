/**
 * @file app_main.cpp
 * @author Zotbins (zotbinsuci@gmail.com)
 * @author Alex Ikeda (ikedaas@uci.edu)
 * @brief 
 * @version 0.1
 * @date 2025-09-20
 * 
 * @copyright Copyright (c) 2025
 * 
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

static const char *TAG = "app_main"; // Tag for ESP logging

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %" PRIu32 " bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    // Initialize system tasks
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

    // System initialization event group initialization
    extern EventGroupHandle_t sys_init_eg; // sys_init_eg is defined in initialization.cpp and must exist for the lifetime of the MQTT program
    initialize(); // create the event group, from initialization.cpp. Other initialization conditions can be added if needed.

    // Connect client to MQTT broker
    client_connect();

    // Wait for MQTT connection to be established
    ESP_LOGI(TAG, "Waiting for client to intialize...");
    xEventGroupWaitBits(sys_init_eg, BIT0, pdTRUE, pdTRUE, portMAX_DELAY);
    ESP_LOGI(TAG, "Client initialized!");

    // Initialize peripheral manager after system initialization is complete, this manages the sensors (peripheral_manager.cpp)
    ESP_LOGI(TAG, "System initialization complete, initializing peripheral manager...");
    init_manager();
    ESP_LOGI(TAG, "Peripheral manager initialized!");

}
