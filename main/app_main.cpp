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

#include "esp_log.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "client_connect.hpp"
#include "client_publish.hpp"

#include "peripheral_manager.hpp"

#include "freertos/event_groups.h"
#include "esp_system.h"
#include "initialization.hpp"

#include "wifi_manager.hpp"

static const char *TAG = "app_main"; // Tag for ESP logging

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %" PRIu32 " bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_ERROR_CHECK(zb_wifi_init());
    ESP_ERROR_CHECK(zb_wifi_start(CONFIG_EXAMPLE_WIFI_SSID, CONFIG_EXAMPLE_WIFI_PASSWORD));

    extern EventGroupHandle_t sys_init_eg;
    initialize();

    // Connect client to MQTT broker
    client_connect();

    // Wait for MQTT connection to be established
    ESP_LOGI(TAG, "Waiting for client to intialize...");
    xEventGroupWaitBits(sys_init_eg, CLIENT_CONNECT_STATUS_EVENT_BIT, pdTRUE, pdTRUE, portMAX_DELAY);
    ESP_LOGI(TAG, "Client initialized!");

    // Initialize peripheral manager after system initialization is complete, this manages the sensors (peripheral_manager.cpp)
    ESP_LOGI(TAG, "System initialization complete, initializing peripheral manager...");

    init_manager();

    if (zb_wifi_is_connected()) {
        client_connect();
    }
    else {
        xTaskCreate(
            [](void *) {
                xEventGroupWaitBits(zb_wifi_eg, ZB_WIFI_CONNECTED_BIT, pdFALSE, pdTRUE, portMAX_DELAY);
                client_connect();
                vTaskDelete(nullptr);
            },
            "mqtt_deferred_connect",
            4096,
            nullptr,
            4,
            nullptr);
    }

    xEventGroupWaitBits(sys_init_eg, BIT0, pdTRUE, pdTRUE, portMAX_DELAY);
}
