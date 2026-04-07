#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <inttypes.h>

#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_wifi.h"
#include "esp_netif_sntp.h"

#define TAG "POWER_SLEEP"

#define NIGHT_START_HOUR 20
#define NIGHT_END_HOUR   8

#define TIME_SYNC_TIMEOUT_S     15
#define TIME_RESYNC_INTERVAL_S  (6 * 60 * 60)

static time_t s_last_sync_epoch = 0;

/* ---------- Timezone ---------- */
void power_sleep_set_timezone_pacific(void)
{
    setenv("TZ", "PST8PDT,M3.2.0/2,M11.1.0/2", 1);
    tzset();
}

/* ---------- Helpers ---------- */
static bool time_is_valid(void)
{
    time_t now;
    time(&now);

    ESP_LOGD(TAG, "epoch=%lld", (long long) now);

    return (now > 1577836800);
}

static bool is_quiet_hours(const struct tm *lt)
{
    return (lt->tm_hour >= NIGHT_START_HOUR ||
            lt->tm_hour < NIGHT_END_HOUR);
}

static uint32_t seconds_until_8am(const struct tm *lt)
{
    struct tm target = *lt;
    target.tm_hour = 8;
    target.tm_min = 0;
    target.tm_sec = 0;

    time_t now_t = mktime((struct tm *)lt);
    time_t target_t = mktime(&target);

    if (lt->tm_hour >= 20) {
        target.tm_mday += 1;
        target_t = mktime(&target);
    }

    int delta = (int)difftime(target_t, now_t);
    if (delta < 60) delta = 60;

    return (uint32_t)delta;
}

/* ---------- SNTP ---------- */
bool power_sleep_sync_time_if_needed(void)
{
    time_t now;
    time(&now);

    if (time_is_valid() &&
        s_last_sync_epoch != 0 &&
        (now - s_last_sync_epoch) < TIME_RESYNC_INTERVAL_S) {

        return true;
    }

    ESP_LOGI(TAG, "Starting SNTP");

    esp_sntp_config_t config =
        ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");

    esp_netif_sntp_init(&config);

    for (int i = 0; i < TIME_SYNC_TIMEOUT_S; i++) {

        if (time_is_valid()) {
            time(&now);
            s_last_sync_epoch = now;

            ESP_LOGI(TAG, "Time synced: %lld", (long long) now);
            return true;
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    ESP_LOGW(TAG, "SNTP timeout");
    return false;
}

/* ---------- Radios ---------- */
void power_sleep_stop_radios(void)
{
    esp_wifi_stop();
}

/* ---------- Deep Sleep ---------- */
bool power_sleep_maybe_enter_night_deep_sleep(void)
{
    time_t now;
    time(&now);

    struct tm local_tm;
    localtime_r(&now, &local_tm);

    if (!time_is_valid()) return false;

    if (is_quiet_hours(&local_tm)) {

        uint32_t sleep_s = seconds_until_8am(&local_tm);

        ESP_LOGI(TAG, "Deep sleep for %" PRIu32 " seconds", sleep_s);

        power_sleep_stop_radios();

        esp_sleep_enable_timer_wakeup(
            (uint64_t)sleep_s * 1000000ULL
        );

        esp_deep_sleep_start();
    }

    return false;
}