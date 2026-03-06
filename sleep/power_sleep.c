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

#define TAG "POWER_SLEEP"

// --------- CONFIG ---------
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASS "YOUR_WIFI_PASSWORD"

#define NIGHT_START_HOUR 20
#define NIGHT_END_HOUR   8

#define TIME_SYNC_TIMEOUT_S     15
#define TIME_RESYNC_INTERVAL_S  (6 * 60 * 60)
// --------------------------

static EventGroupHandle_t s_wifi_event_group;
static const int WIFI_CONNECTED_BIT = BIT0;

static bool s_netif_inited = false;
static time_t s_last_sync_epoch = 0;


// ---------- Timezone ----------
void power_sleep_set_timezone_pacific(void)
{
    ESP_LOGI(TAG, "Setting timezone to Pacific with DST");

    setenv("TZ", "PST8PDT,M3.2.0/2,M11.1.0/2", 1);
    tzset();

    ESP_LOGI(TAG, "Timezone configured");
}


// ---------- Helpers ----------
static bool time_is_valid(void)
{
    time_t now = 0;
    time(&now);

    ESP_LOGD(TAG, "Checking time validity: epoch=%ld", now);

    bool valid = (now > 1577836800);

    ESP_LOGD(TAG, "Time valid: %s", valid ? "YES" : "NO");

    return valid;
}


static bool is_quiet_hours_8pm_to_8am(const struct tm *lt)
{
    ESP_LOGD(TAG, "Checking quiet hours. Hour=%d", lt->tm_hour);

    bool quiet =
        (lt->tm_hour >= NIGHT_START_HOUR) ||
        (lt->tm_hour < NIGHT_END_HOUR);

    ESP_LOGI(TAG,
        "Quiet hours check: hour=%d -> %s",
        lt->tm_hour,
        quiet ? "YES" : "NO");

    return quiet;
}


static uint32_t seconds_until_next_8am(const struct tm *lt)
{
    ESP_LOGI(TAG,
        "Calculating seconds until next 8AM. Current time: %02d:%02d:%02d",
        lt->tm_hour,
        lt->tm_min,
        lt->tm_sec);

    struct tm target = *lt;

    target.tm_hour = 8;
    target.tm_min  = 0;
    target.tm_sec  = 0;

    time_t now_t = mktime((struct tm *)lt);
    time_t target_t = mktime(&target);

    if (lt->tm_hour >= 20) {
        ESP_LOGI(TAG, "After 8PM → scheduling wake for tomorrow 8AM");
        target.tm_mday += 1;
        target_t = mktime(&target);
    }
    else if (lt->tm_hour < 8) {
        ESP_LOGI(TAG, "Before 8AM → scheduling wake for today 8AM");
    }
    else {
        ESP_LOGI(TAG, "Daytime fallback → scheduling tomorrow 8AM");
        target.tm_mday += 1;
        target_t = mktime(&target);
    }

    int delta = (int)difftime(target_t, now_t);

    if (delta < 60) {
        ESP_LOGW(TAG, "Sleep delta too small, forcing 60 seconds");
        delta = 60;
    }

    ESP_LOGI(TAG, "Sleep duration until 8AM: %d seconds", delta);

    return (uint32_t)delta;
}


// ---------- Wi-Fi events ----------
static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    (void)arg;
    (void)event_data;

    ESP_LOGI(TAG,
        "WiFi event received: base=%s id=%ld",
        event_base,
        event_id);

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(TAG, "WiFi started → attempting connection");
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGW(TAG, "WiFi disconnected → retrying");
        esp_wifi_connect();
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ESP_LOGI(TAG, "WiFi connected and IP obtained");
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}


// ---------- Wi-Fi connect ----------
static esp_err_t wifi_connect_blocking(uint32_t timeout_ms)
{
    ESP_LOGI(TAG, "Starting WiFi connection (timeout=%d ms)", timeout_ms);

    s_wifi_event_group = xEventGroupCreate();

    if (!s_netif_inited) {

        ESP_LOGI(TAG, "Initializing network stack");

        ESP_ERROR_CHECK(esp_netif_init());
        ESP_ERROR_CHECK(esp_event_loop_create_default());

        esp_netif_create_default_wifi_sta();

        s_netif_inited = true;
    }

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    ESP_LOGI(TAG, "Initializing WiFi driver");

    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_LOGI(TAG, "Registering WiFi event handlers");

    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL));


    wifi_config_t wifi_config = {0};

    strncpy((char *)wifi_config.sta.ssid, WIFI_SSID, sizeof(wifi_config.sta.ssid));
    strncpy((char *)wifi_config.sta.password, WIFI_PASS, sizeof(wifi_config.sta.password));

    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

    ESP_LOGI(TAG, "Setting WiFi mode STA");

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    ESP_LOGI(TAG, "Applying WiFi credentials");

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

    ESP_LOGI(TAG, "Starting WiFi");

    ESP_ERROR_CHECK(esp_wifi_start());


    ESP_LOGI(TAG, "Waiting for WiFi connection...");

    EventBits_t bits = xEventGroupWaitBits(
        s_wifi_event_group,
        WIFI_CONNECTED_BIT,
        pdFALSE,
        pdTRUE,
        pdMS_TO_TICKS(timeout_ms)
    );

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "WiFi connection successful");
        return ESP_OK;
    }

    ESP_LOGW(TAG, "WiFi connection timed out");

    return ESP_ERR_TIMEOUT;
}


// ---------- SNTP ----------
bool power_sleep_sync_time_if_needed(void)
{
    time_t now;
    time(&now);

    ESP_LOGI(TAG, "Checking if time sync is needed");

    if (time_is_valid() && s_last_sync_epoch != 0 &&
        (now - s_last_sync_epoch) < TIME_RESYNC_INTERVAL_S) {

        ESP_LOGI(TAG, "Recent time sync detected → skipping");

        return true;
    }

    ESP_LOGI(TAG, "Starting SNTP time synchronization");

    if (wifi_connect_blocking(15000) != ESP_OK) {

        ESP_LOGW(TAG, "WiFi connection failed → cannot sync time");

        return false;
    }

    ESP_LOGI(TAG, "Initializing SNTP client");

    esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");

    esp_netif_sntp_init(&config);

    ESP_LOGI(TAG, "Waiting for SNTP response");

    for (int i = 0; i < TIME_SYNC_TIMEOUT_S; i++) {

        if (time_is_valid()) {

            time(&now);

            s_last_sync_epoch = now;

            ESP_LOGI(TAG, "SNTP time sync successful (epoch=%ld)", now);

            return true;
        }

        ESP_LOGI(TAG, "Waiting for SNTP... (%d/%d)", i+1, TIME_SYNC_TIMEOUT_S);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    ESP_LOGW(TAG, "SNTP sync timed out");

    return false;
}


// ---------- Stop radios ----------
void power_sleep_stop_radios(void)
{
    ESP_LOGI(TAG, "Stopping WiFi radios");

    esp_wifi_stop();

    ESP_LOGI(TAG, "Radios stopped");
}


// ---------- Night policy ----------
bool power_sleep_maybe_enter_night_deep_sleep(void)
{
    time_t now;
    time(&now);

    struct tm local_tm;

    localtime_r(&now, &local_tm);

    ESP_LOGI(TAG,
        "Checking night sleep policy. Local time: %02d:%02d:%02d",
        local_tm.tm_hour,
        local_tm.tm_min,
        local_tm.tm_sec);

    if (!time_is_valid()) {

        ESP_LOGW(TAG, "Time invalid → cannot determine quiet hours");

        return false;
    }

    if (is_quiet_hours_8pm_to_8am(&local_tm)) {

        uint32_t sleep_s = seconds_until_next_8am(&local_tm);

        ESP_LOGI(TAG,
            "Quiet hours detected → deep sleeping until 8AM (%u seconds)",
            sleep_s);

        power_sleep_stop_radios();

        ESP_LOGI(TAG, "Configuring deep sleep timer");

        ESP_ERROR_CHECK(
            esp_sleep_enable_timer_wakeup(
                (uint64_t)sleep_s * 1000000ULL
            )
        );

        ESP_LOGI(TAG, "Entering deep sleep now");

        esp_deep_sleep_start();
    }

    ESP_LOGI(TAG, "Not within quiet hours → continuing normal operation");

    return false;
}