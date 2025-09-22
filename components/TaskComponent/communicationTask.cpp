// #include "driver/gpio.h"
// #include "esp_event.h"
// #include "esp_log.h"
// #include "esp_mac.h"
// #include "esp_netif.h"
// #include "esp_now.h"
// #include "esp_wifi.h"
// #include "freertos/FreeRTOS.h"
// #include "freertos/queue.h"
// #include "nvs_flash.h"
// #include <stdlib.h>
// #include <string.h>

// #include "Client.hpp"
// #include "Serialize.hpp"
// #include "communicationTask.hpp"

// using namespace Zotbins;

// static const char *name = "communicationTask";
// static const int priority = 1;
// static const uint32_t stackSize = 4096;
// static TaskHandle_t usageHandle = NULL;
// static const int core = 1;

// #define ESPNOW_MAXDELAY 512
// #define CONFIG_ESPNOW_CHANNEL 1
// #define ESPNOW_WIFI_MODE WIFI_MODE_STA
// #define ESPNOW_WIFI_IF WIFI_IF_STA

// // Fallback definition for MACSTR if not provided by esp_mac.h
// #ifndef MACSTR
// #define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"
// #endif

// static QueueHandle_t s_espnow_queue = NULL;
// static uint8_t s_broadcast_mac[ESP_NOW_ETH_ALEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// /* ESP-NOW send callback */
// static void espnow_send_cb(const uint8_t *mac_addr, esp_now_send_status_t status)
// {
//     if (mac_addr == NULL)
//     {
//         ESP_LOGE(name, "Send callback error");
//         return;
//     }
//     char mac_str[18]; // Buffer for MAC address string (17 chars + null terminator)
//     snprintf(mac_str, sizeof(mac_str), MACSTR, MAC2STR(mac_addr));
//     ESP_LOGI(name, "Sent to %s, status: %s", mac_str, status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
// }

// /* ESP-NOW receive callback */
// static void espnow_recv_cb(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len)
// {
//     if (recv_info->src_addr == NULL || data == NULL || len <= 0)
//     {
//         ESP_LOGE(name, "Receive callback error");
//         return;
//     }

//     char *received_msg = (char *)malloc(len + 1);
//     if (received_msg == NULL)
//     {
//         ESP_LOGE(name, "Malloc receive buffer fail");
//         return;
//     }
//     memcpy(received_msg, data, len);
//     received_msg[len] = '\0'; // Null-terminate string
//     char mac_str[18];         // Buffer for MAC address string (17 chars + null terminator)
//     snprintf(mac_str, sizeof(mac_str), MACSTR, MAC2STR(recv_info->src_addr));
//     ESP_LOGI(name, "Received from %s: %s", mac_str, received_msg);
//     free(received_msg);
// }

// /* ESP-NOW task to send the message */
// // void espnow_send_message(const char *message) {
// //     uint32_t message_len = strlen(message) + 1; // Include null terminator

// //     ESP_LOGI(name, "Start sending: %s", message);

// //     // Send the message using ESP-NOW
// //     if (esp_now_send(s_broadcast_mac, (uint8_t *)message, message_len) != ESP_OK) {
// //         ESP_LOGE(name, "Send error");
// //     }
// // }

// /* Initialize ESP-NOW */
// static esp_err_t espnow_init(void)
// {
//     s_espnow_queue = xQueueCreate(10, sizeof(uint32_t)); // Simplified queue for events
//     if (s_espnow_queue == NULL)
//     {
//         ESP_LOGE(name, "Create queue fail");
//         return ESP_FAIL;
//     }

//     ESP_ERROR_CHECK(esp_now_init());
//     ESP_ERROR_CHECK(esp_now_register_send_cb(espnow_send_cb));
//     ESP_ERROR_CHECK(esp_now_register_recv_cb(espnow_recv_cb));

//     // Add broadcast peer
//     esp_now_peer_info_t *peer = (esp_now_peer_info_t *)malloc(sizeof(esp_now_peer_info_t));
//     if (peer == NULL)
//     {
//         ESP_LOGE(name, "Malloc peer info fail");
//         vQueueDelete(s_espnow_queue);
//         esp_now_deinit();
//         return ESP_FAIL;
//     }
//     memset(peer, 0, sizeof(esp_now_peer_info_t));
//     peer->channel = CONFIG_ESPNOW_CHANNEL;
//     peer->ifidx = ESPNOW_WIFI_IF;
//     peer->encrypt = false;
//     memcpy(peer->peer_addr, s_broadcast_mac, ESP_NOW_ETH_ALEN);
//     ESP_ERROR_CHECK(esp_now_add_peer(peer));
//     free(peer);

//     // xTaskCreate(espnow_task, "espnow_task", 2048, NULL, 4, NULL);
//     return ESP_OK;
// }

// CommunicationTask::CommunicationTask(QueueHandle_t &messageQueue)
//     : Task(name, priority, stackSize), mMessageQueue(messageQueue)
// {
// }

// void CommunicationTask::start()
// {
//     xTaskCreatePinnedToCore(taskFunction, name, stackSize, this, priority, &usageHandle, core);
// }

// void CommunicationTask::taskFunction(void *task)
// {
//     CommunicationTask *communicationTask = static_cast<CommunicationTask *>(task);
//     communicationTask->setup();
//     communicationTask->loop();
// }

// void CommunicationTask::sendMessage(const char *message)
// {
//     uint32_t message_len = strlen(message) + 1; // Include null terminator

//     ESP_LOGI(name, "Start sending: %s", message);

//     // Send the message using ESP-NOW
//     if (esp_now_send(s_broadcast_mac, (uint8_t *)message, message_len) != ESP_OK)
//     {
//         ESP_LOGE(name, "Send error");
//     }
// }

// void CommunicationTask::setup()
// {
//     ESP_LOGE(name, "Hello from Communication Task");

//     espnow_init();

//     while (1)
//     {
//         vTaskDelay(1000 / portTICK_PERIOD_MS);
//         // espnow_send_message("Hello from ESPNow!");
//     }
// }

// void CommunicationTask::loop()
// {
// }
