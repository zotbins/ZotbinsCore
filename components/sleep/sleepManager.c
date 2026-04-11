#include "sleepManager.h"
#include "power_sleep.h"

#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_timer.h"

#define TAG "SLEEP_MGR"

// ====== CONFIG ======
#define BREAKBEAM_GPIO        GPIO_NUM_27
#define ACTIVITY_WINDOW_MS    (15 * 60 * 1000)
#define SHORT_NAP_MINUTES     5
#define LONG_NAP_MINUTES      10
// ====================

static int64_t last_activity_time = 0;
static int inactivity_level = 0;

/* ---------- ISR ---------- */
static void IRAM_ATTR breakbeam_isr(void *arg)
{
    ESP_EARLY_LOGI(TAG, "Breakbeam interrupt");
    record_breakbeam_activity();
}

/* ---------- Activity ---------- */
void record_breakbeam_activity(void)
{
    int64_t now = esp_timer_get_time() / 1000;

    ESP_LOGI(TAG, "Activity detected. Reset inactivity.");

    last_activity_time = now;
    inactivity_level = 0;

    ESP_LOGI(TAG, "last_activity_time = %lld ms", (long long) now);
}

/* ---------- Light Sleep ---------- */
static void take_light_nap(uint32_t minutes)
{
    ESP_LOGI(TAG, "Light sleep for %d minutes", (int)minutes);

    power_sleep_stop_radios();

    esp_sleep_enable_ext0_wakeup(BREAKBEAM_GPIO, 0);

    uint64_t sleep_time =
        (uint64_t)minutes * 60ULL * 1000000ULL;

    ESP_LOGI(TAG, "Timer wakeup: %llu us", sleep_time);

    esp_sleep_enable_timer_wakeup(sleep_time);

    ESP_LOGI(TAG, "Entering light sleep");
    esp_light_sleep_start();

    ESP_LOGI(TAG, "Woke from light sleep");
}

/* ---------- Task ---------- */
static void sleep_manager_task(void *arg)
{
    ESP_LOGI(TAG, "Sleep manager started");

    while (true) {

        int64_t now_ms = esp_timer_get_time() / 1000;
        int64_t inactivity_time = now_ms - last_activity_time;

        ESP_LOGI(TAG, "Inactivity: %lld ms", (long long)inactivity_time);

        power_sleep_sync_time_if_needed();
        power_sleep_maybe_enter_night_deep_sleep();

        if (inactivity_time > ACTIVITY_WINDOW_MS) {

            inactivity_level++;

            if (inactivity_level == 1) {
                take_light_nap(SHORT_NAP_MINUTES);
            } else {
                take_light_nap(LONG_NAP_MINUTES);
            }
        }

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