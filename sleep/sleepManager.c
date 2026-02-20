#include "sleep_manager.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include "esp_sntp.h"
#include "time.h"

#define TAG "SLEEP_MGR"

// ====== CONFIG ======
#define BREAKBEAM_GPIO        GPIO_NUM_27
#define ACTIVITY_WINDOW_MS    (15 * 60 * 1000)
#define SHORT_NAP_MINUTES     5
#define LONG_NAP_MINUTES      10

#define NIGHT_START_HOUR      20  // 8 PM
#define NIGHT_END_HOUR        8   // 8 AM

#define TIME_SYNC_INTERVAL_S  (6 * 60 * 60)
// ====================

static int64_t last_activity_time = 0;
static int inactivity_level = 0;
static int64_t last_time_sync = 0;

/* ---------- Breakbeam ISR ---------- */
static void IRAM_ATTR breakbeam_isr(void *arg)
{
    record_breakbeam_activity();
}

/* ---------- Public API ---------- */
void record_breakbeam_activity(void)
{
    last_activity_time = esp_timer_get_time() / 1000; // ms
    inactivity_level = 0;
}

/* ---------- Time Helpers ---------- */
static bool is_night_time(void)
{
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);

    int hour = timeinfo.tm_hour;
    return (hour >= NIGHT_START_HOUR || hour < NIGHT_END_HOUR);
}

/* ---------- WiFi Time Sync ---------- */
static void sync_time_if_needed(void)
{
    time_t now;
    time(&now);

    if ((now - last_time_sync) < TIME_SYNC_INTERVAL_S) {
        return;
    }

    ESP_LOGI(TAG, "Syncing time via SNTP");

    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_init();

    // wait briefly for sync
    vTaskDelay(pdMS_TO_TICKS(2000));

    time(&now);
    last_time_sync = now;

    esp_sntp_stop();
}

/* ---------- Light Sleep ---------- */
static void take_light_nap(uint32_t minutes)
{
    ESP_LOGI(TAG, "Entering light sleep for %d minutes", minutes);

    esp_wifi_stop();

    esp_sleep_enable_ext0_wakeup(BREAKBEAM_GPIO, 0);
    esp_sleep_enable_timer_wakeup(
        (uint64_t)minutes * 60ULL * 1000000ULL
    );

    esp_light_sleep_start();

    ESP_LOGI(TAG, "Woke from light sleep");
}

/* ---------- Deep Sleep (Night) ---------- */
static void night_deep_sleep(void)
{
    ESP_LOGI(TAG, "Night mode: deep sleep");

    // Wake at 8 AM
    uint64_t sleep_time_us = 12ULL * 60ULL * 60ULL * 1000000ULL;
    esp_sleep_enable_timer_wakeup(sleep_time_us);
    esp_deep_sleep_start();
}

/* ---------- Main Sleep Task ---------- */
void sleep_manager_task(void *arg)
{
    while (true) {
        int64_t now_ms = esp_timer_get_time() / 1000;

        if (is_night_time()) {
            night_deep_sleep();
        }

        if ((now_ms - last_activity_time) > ACTIVITY_WINDOW_MS) {
            inactivity_level++;

            if (inactivity_level == 1) {
                take_light_nap(SHORT_NAP_MINUTES);
            } else {
                take_light_nap(LONG_NAP_MINUTES);
            }

            sync_time_if_needed();
        }

        vTaskDelay(pdMS_TO_TICKS(60000)); // check every minute
    }
}

/* ---------- Init ---------- */
void sleep_manager_init(void)
{
    ESP_LOGI(TAG, "Initializing Sleep Manager");

    gpio_config_t io_conf = {
        .pin_bit_mask = 1ULL << BREAKBEAM_GPIO,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .intr_type = GPIO_INTR_NEGEDGE
    };
    gpio_config(&io_conf);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(BREAKBEAM_GPIO, breakbeam_isr, NULL);

    last_activity_time = esp_timer_get_time() / 1000;

    xTaskCreate(
        sleep_manager_task,
        "sleep_manager_task",
        4096,
        NULL,
        5,
        NULL
    );
}
