// components/power_management/power_sleep.c
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"

#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_sntp.h"
#include "esp_sleep.h"
#include "esp_wifi.h"

// If you have a header, include it instead.
// #include "power_sleep.h"

#define TAG "POWER_SLEEP"

// --------- CONFIG ---------
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASS "YOUR_WIFI_PASSWORD"

#define NIGHT_START_HOUR 20  // 8pm
#define NIGHT_END_HOUR   8   // 8am

#define TIME_SYNC_TIMEOUT_S     15
#define TIME_RESYNC_INTERVAL_S  (6 * 60 * 60)  // 6 hours
// --------------------------

static EventGroupHandle_t s_wifi_event_group;
static const int WIFI_CONNECTED_BIT = BIT0;

static bool s_netif_inited = false;
static time_t s_last_sync_epoch = 0;

// ---------- Timezone: Pacific w/ DST ----------
void power_sleep_set_timezone_pacific(void)
{
    // PST/PDT with US DST rules
    setenv("TZ", "PST8PDT,M3.2.0/2,M11.1.0/2", 1);
    tzset();
}

// ---------- Helpers ----------
static bool time_is_valid(void)
{
    time_t now = 0;
    time(&now);
    return (now > 1577836800); // > 2020-01-01
}

static bool is_quiet_hours_8pm_to_8am(const struct tm *lt)
{
    return (lt->tm_hour >= NIGHT_START_HOUR) || (lt->tm_hour < NIGHT_END_HOUR);
}

static uint32_t seconds_until_next_8am(const struct tm *lt)
{
    struct tm target = *lt;
    target.tm_hour = 8;
    target.tm_min  = 0;
    target.tm_sec  = 0;

    time_t now_t = mktime((struct tm *)lt);
    time_t target_t = mktime(&target);

    if (lt->tm_hour >= 20) {
        target.tm_mday += 1; // tomorrow 8am
        target_t = mktime(&target);
    } else if (lt->tm_hour < 8) {
        // today 8am (already set)
    } else {
        // daytime fallback: tomorrow 8am
        target.tm_mday += 1;
        target_t = mktime(&target);
    }

    int delta = (int)difftime(target_t, now_t);
    if (delta < 60) delta = 60;
    return (uint32_t)delta;
}

// ---------- Wi-Fi connect (blocking) ----------
static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    (void)arg; (void)event_data;

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

static esp_err_t wifi_connect_blocking(uint32_t timeout_ms)
{
    s_wifi_event_group = xEventGroupCreate();

    if (!s_netif_inited) {
        ESP_ERROR_CHECK(esp_netif_init());
        ESP_ERROR_CHECK(esp_event_loop_create_default());
        esp_netif_create_default_wifi_sta();
        s_netif_inited = true;
    }

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL));

    wifi_config_t wifi_config = {0};
    strncpy((char *)wifi_config.sta.ssid, WIFI_SSID, sizeof(wifi_config.sta.ssid));
    strncpy((char *)wifi_config.sta.password, WIFI_PASS, sizeof(wifi_config.sta.password));
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    EventBits_t bits = xEventGroupWaitBits(
        s_wifi_event_group,
        WIFI_CONNECTED_BIT,
        pdFALSE,
        pdTRUE,
        pdMS_TO_TICKS(timeout_ms)
    );

    return (bits & WIFI_CONNECTED_BIT) ? ESP_OK : ESP_ERR_TIMEOUT;
}

// ---------- SNTP ----------
bool power_sleep_sync_time_if_needed(void)
{
    time_t now;
    time(&now);

    // If time valid and we recently synced, skip
    if (time_is_valid() && s_last_sync_epoch != 0 &&
        (now - s_last_sync_epoch) < TIME_RESYNC_INTERVAL_S) {
        return true;
    }

    ESP_LOGI(TAG, "Syncing time via SNTP...");
    if (wifi_connect_blocking(15000) != ESP_OK) {
        ESP_LOGW(TAG, "Wi-Fi connect timed out; cannot sync time");
        return false;
    }

    esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");
    esp_netif_sntp_init(&config);

    for (int i = 0; i < TIME_SYNC_TIMEOUT_S; i++) {
        if (time_is_valid()) {
            time(&now);
            s_last_sync_epoch = now;
            ESP_LOGI(TAG, "Time synced OK");
            return true;
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    ESP_LOGW(TAG, "SNTP sync timed out");
    return false;
}

void power_sleep_stop_radios(void)
{
    // Ignore errors if not started
    esp_wifi_stop();
}

// ---------- Night policy ----------
bool power_sleep_maybe_enter_night_deep_sleep(void)
{
    time_t now;
    time(&now);

    // Need timezone set before localtime_r
    struct tm local_tm;
    localtime_r(&now, &local_tm);

    if (!time_is_valid()) {
        return false; // caller decides what to do if no time
    }

    if (is_quiet_hours_8pm_to_8am(&local_tm)) {
        uint32_t sleep_s = seconds_until_next_8am(&local_tm);

        ESP_LOGI(TAG, "Quiet hours (PT). Deep sleep until 8am (%u sec)", (unsigned)sleep_s);

        power_sleep_stop_radios();
        ESP_ERROR_CHECK(esp_sleep_enable_timer_wakeup((uint64_t)sleep_s * 1000000ULL));
        esp_deep_sleep_start(); // no return
    }

    return false;
}
