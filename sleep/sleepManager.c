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

#define NIGHT_START_HOUR      20
#define NIGHT_END_HOUR        8

#define TIME_SYNC_INTERVAL_S  (6 * 60 * 60)
// ====================

static int64_t last_activity_time = 0;
static int inactivity_level = 0;
static int64_t last_time_sync = 0;

/* ---------- Breakbeam ISR ---------- */
static void IRAM_ATTR breakbeam_isr(void *arg)
{
    ESP_EARLY_LOGI(TAG, "Breakbeam interrupt triggered");
    record_breakbeam_activity();
}

/* ---------- Public API ---------- */
void record_breakbeam_activity(void)
{
    int64_t now = esp_timer_get_time() / 1000;

    ESP_LOGI(TAG,
        "Breakbeam activity detected. Resetting inactivity timer. Previous inactivity level: %d",
        inactivity_level);

    last_activity_time = now;
    inactivity_level = 0;

    ESP_LOGI(TAG, "New last_activity_time = %lld ms", now);
}

/* ---------- Time Helpers ---------- */
static bool is_night_time(void)
{
    time_t now;
    struct tm timeinfo;

    time(&now);
    localtime_r(&now, &timeinfo);

    int hour = timeinfo.tm_hour;

    ESP_LOGI(TAG,
        "Current time: %02d:%02d:%02d",
        timeinfo.tm_hour,
        timeinfo.tm_min,
        timeinfo.tm_sec);

    bool night = (hour >= NIGHT_START_HOUR || hour < NIGHT_END_HOUR);

    ESP_LOGI(TAG,
        "Night check -> hour=%d night=%s",
        hour,
        night ? "YES" : "NO");

    return night;
}

/* ---------- WiFi Time Sync ---------- */
static void sync_time_if_needed(void)
{
    time_t now;
    time(&now);

    ESP_LOGI(TAG,
        "Checking if time sync needed. Last sync: %lld Current: %ld",
        last_time_sync,
        now);

    if ((now - last_time_sync) < TIME_SYNC_INTERVAL_S) {
        ESP_LOGI(TAG, "Time sync not required");
        return;
    }

    ESP_LOGI(TAG, "Starting SNTP time sync");

    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_init();

    ESP_LOGI(TAG, "Waiting for time sync...");
    vTaskDelay(pdMS_TO_TICKS(2000));

    time(&now);
    last_time_sync = now;

    ESP_LOGI(TAG, "Time synchronized. New timestamp: %ld", now);

    esp_sntp_stop();

    ESP_LOGI(TAG, "SNTP stopped");
}

/* ---------- Light Sleep ---------- */
static void take_light_nap(uint32_t minutes)
{
    ESP_LOGI(TAG,
        "Preparing for light sleep. Duration: %d minutes",
        minutes);

    ESP_LOGI(TAG, "Stopping WiFi before sleep");
    esp_wifi_stop();

    ESP_LOGI(TAG,
        "Enabling breakbeam wakeup on GPIO %d",
        BREAKBEAM_GPIO);

    esp_sleep_enable_ext0_wakeup(BREAKBEAM_GPIO, 0);

    uint64_t sleep_time =
        (uint64_t)minutes * 60ULL * 1000000ULL;

    ESP_LOGI(TAG,
        "Enabling timer wakeup: %llu us",
        sleep_time);

    esp_sleep_enable_timer_wakeup(sleep_time);

    ESP_LOGI(TAG, "Entering light sleep now");
    esp_light_sleep_start();

    ESP_LOGI(TAG, "Device woke from light sleep");

    esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();

    ESP_LOGI(TAG, "Wakeup cause: %d", cause);
}

/* ---------- Deep Sleep (Night) ---------- */
static void night_deep_sleep(void)
{
    ESP_LOGI(TAG, "Night mode triggered -> entering deep sleep");

    uint64_t sleep_time_us =
        12ULL * 60ULL * 60ULL * 1000000ULL;

    ESP_LOGI(TAG,
        "Deep sleep duration: %llu us (~12 hours)",
        sleep_time_us);

    esp_sleep_enable_timer_wakeup(sleep_time_us);

    ESP_LOGI(TAG, "Entering deep sleep now");
    esp_deep_sleep_start();
}

/* ---------- Main Sleep Task ---------- */
void sleep_manager_task(void *arg)
{
    ESP_LOGI(TAG, "Sleep manager task started");

    while (true) {

        int64_t now_ms = esp_timer_get_time() / 1000;

        ESP_LOGI(TAG,
            "Sleep manager loop tick. Current time(ms): %lld",
            now_ms);

        int64_t inactivity_time = now_ms - last_activity_time;

        ESP_LOGI(TAG,
            "Inactivity duration: %lld ms",
            inactivity_time);

        if (is_night_time()) {
            ESP_LOGI(TAG, "Night detected -> entering deep sleep");
            night_deep_sleep();
        }

        if (inactivity_time > ACTIVITY_WINDOW_MS) {

            inactivity_level++;

            ESP_LOGW(TAG,
                "Inactivity window exceeded. Level=%d",
                inactivity_level);

            if (inactivity_level == 1) {

                ESP_LOGI(TAG,
                    "Taking short nap (%d minutes)",
                    SHORT_NAP_MINUTES);

                take_light_nap(SHORT_NAP_MINUTES);

            } else {

                ESP_LOGI(TAG,
                    "Taking long nap (%d minutes)",
                    LONG_NAP_MINUTES);

                take_light_nap(LONG_NAP_MINUTES);
            }

            ESP_LOGI(TAG, "Checking if time sync required");
            sync_time_if_needed();
        }

        ESP_LOGI(TAG,
            "Sleep manager waiting 60 seconds before next check");

        vTaskDelay(pdMS_TO_TICKS(60000));
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

    ESP_LOGI(TAG,
        "Configuring breakbeam GPIO: %d",
        BREAKBEAM_GPIO);

    gpio_config(&io_conf);

    ESP_LOGI(TAG, "Installing GPIO ISR service");
    gpio_install_isr_service(0);

    ESP_LOGI(TAG, "Attaching ISR to breakbeam GPIO");
    gpio_isr_handler_add(BREAKBEAM_GPIO, breakbeam_isr, NULL);

    last_activity_time = esp_timer_get_time() / 1000;

    ESP_LOGI(TAG,
        "Initial last_activity_time = %lld",
        last_activity_time);

    ESP_LOGI(TAG, "Creating sleep manager task");

    xTaskCreate(
        sleep_manager_task,
        "sleep_manager_task",
        4096,
        NULL,
        5,
        NULL
    );
}