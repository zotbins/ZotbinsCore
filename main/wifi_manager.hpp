#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C"
{
#endif
    extern EventGroupHandle_t zb_wifi_eg;
#ifdef __cplusplus
}
#endif

constexpr EventBits_t ZB_WIFI_CONNECTED_BIT = BIT0;

esp_err_t zb_wifi_init(void);
esp_err_t zb_wifi_start(const char *ssid, const char *pass);
esp_err_t zb_wifi_stop(void);
bool zb_wifi_is_connected(void);