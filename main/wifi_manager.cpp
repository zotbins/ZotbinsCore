#include "wifi_manager.hpp"

#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"

#include <atomic>

static const char *TAG = "zb_wifi";
static esp_netif_t *s_netif = nullptr;
static esp_event_handler_instance_t s_wifi_any = nullptr;
static esp_event_handler_instance_t s_ip_got = nullptr;
static TaskHandle_t s_watchdog = nullptr;

EventGroupHandle_t zb_wifi_eg = nullptr;
static std::atomic<bool> s_connected{false};

#ifndef ZB_WIFI_RETRY_INTERVAL_MS
#define ZB_WIFI_RETRY_INTERVAL_MS (10 * 1000)
#endif

static void wifi_evt(void *,
                     esp_event_base_t base,
                     int32_t id,
                     void *data)
{
    if (base == WIFI_EVENT)
    {
        switch (id)
        {
        case WIFI_EVENT_STA_START:
            esp_wifi_connect();
            break;
        case WIFI_EVENT_STA_CONNECTED:
            break;
        case WIFI_EVENT_STA_DISCONNECTED:
            s_connected.store(false, std::memory_order_relaxed);
            xEventGroupClearBits(zb_wifi_eg, ZB_WIFI_CONNECTED_BIT);
            break;
        default:
            break;
        }
    }
    else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP)
    {
        auto *e = static_cast<ip_event_got_ip_t *>(data);
        ESP_LOGI(TAG, "got ip: " IPSTR, IP2STR(&e->ip_info.ip));
        s_connected.store(true, std::memory_order_relaxed);
        xEventGroupSetBits(zb_wifi_eg, ZB_WIFI_CONNECTED_BIT);
    }
}

static void watchdog_task(void *)
{
    for (;;)
    {
        if (!s_connected.load(std::memory_order_relaxed))
        {
            ESP_LOGI(TAG, "reconnect");
            esp_wifi_connect();
        }
        vTaskDelay(pdMS_TO_TICKS(ZB_WIFI_RETRY_INTERVAL_MS));
    }
}

esp_err_t zb_wifi_init(void)
{
    if (!zb_wifi_eg)
        zb_wifi_eg = xEventGroupCreate();

    if (!s_netif)
    {
        s_netif = esp_netif_create_default_wifi_sta();
    }

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_evt, nullptr, &s_wifi_any));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_evt, nullptr, &s_ip_got));

    return ESP_OK;
}

esp_err_t zb_wifi_start(const char *ssid, const char *pass)
{
    wifi_config_t conf = {};
    snprintf(reinterpret_cast<char *>(conf.sta.ssid), sizeof(conf.sta.ssid), "%s", ssid);
    snprintf(reinterpret_cast<char *>(conf.sta.password), sizeof(conf.sta.password), "%s", pass);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &conf));
    ESP_ERROR_CHECK(esp_wifi_start());

    if (!s_watchdog)
    {
        xTaskCreatePinnedToCore(watchdog_task, "zb_wifi_watchdog",
                                4096, nullptr, 4, &s_watchdog, tskNO_AFFINITY);
    }

    ESP_LOGI(TAG, "sta started");
    return ESP_OK;
}

esp_err_t zb_wifi_stop(void)
{
    if (zb_wifi_eg)
    {
        vEventGroupDelete(zb_wifi_eg);
        zb_wifi_eg = nullptr;
    }
    esp_err_t r = esp_wifi_disconnect();
    if (r != ESP_OK)
        return r;

    r = esp_wifi_stop();
    if (r == ESP_ERR_WIFI_NOT_INIT)
        return r;

    ESP_ERROR_CHECK(esp_wifi_deinit());
    if (s_netif)
    {
        ESP_ERROR_CHECK(esp_wifi_clear_default_wifi_driver_and_handlers(s_netif));
        esp_netif_destroy(s_netif);
        s_netif = nullptr;
    }
    return ESP_OK;
}

bool zb_wifi_is_connected(void)
{
    return s_connected.load(std::memory_order_relaxed);
}