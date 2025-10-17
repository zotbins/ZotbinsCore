#include <stdint.h>

#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_log.h"

#include "servo.hpp"

/*
    GPIO Pin Configuation
    DO NOT declare pin numbers as static to avoid duplicate pin assignments.
    Please refer to the ESP32-WROVER datasheet for the pinouts.
    https://www.espressif.com/sites/default/files/documentation/esp32-wrover-e_esp32-wrover-ie_datasheet_en.pdf
*/
const gpio_num_t PIN_SERVO = GPIO_NUM_25; // Pin 25 chosen due to availability constraints, no boot conflicts, same power domain as other peripherals, and commonly used for DAC, PWM, LED, and servo examples in ESP-IDF

// NECESSARY for some pins. Always safer to use a gpio_config_t to configure pins.
static const gpio_config_t PIN_SERVO_CONFIG = {
    .pin_bit_mask = (1ULL << PIN_SERVO),
    .mode = GPIO_MODE_OUTPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE};

static const char *TAG = "servo";

// LEDC configuration for servo (50 Hz)
static constexpr int kFreqHz = 50;                    // 50 Hz
static constexpr int kResolution = LEDC_TIMER_16_BIT; // good granularity
static constexpr int kPeriodUs = 1000000 / kFreqHz;   // 20,000 µs

// Typical pulse bounds
static constexpr int kMinUs = 500;  // ~0°
static constexpr int kMaxUs = 2500; // ~180°

// Use a single low-speed LEDC timer and one channel for this servo.
static constexpr ledc_timer_t kTimer = LEDC_TIMER_0;
static constexpr ledc_mode_t kMode = LEDC_LOW_SPEED_MODE;
static constexpr ledc_channel_t kChannel = LEDC_CHANNEL_0;

static inline uint32_t us_to_duty(uint32_t us)
{
    const uint32_t max_duty = (1u << kResolution) - 1u;
    return (uint32_t)((uint64_t)us * max_duty / kPeriodUs);
}

esp_err_t init_servo(void)
{
    esp_err_t err;

    ESP_LOGI(TAG, "Initializing servo...");

    // ABSOLUTELY NECESSARY FOR SOME PINS, COMPLETELY OVERRIDES PREVIOUS CONFIGURATION
    err = gpio_config(&PIN_SERVO_CONFIG);
    if (err != ESP_OK)
    {
        ESP_LOGW(TAG, "gpio_config failed: %s", esp_err_to_name(err));
        return err;
    }

    // Timer (shared across channels)
    ledc_timer_config_t timer = {};
    timer.speed_mode = kMode;
    timer.timer_num = kTimer;
    timer.duty_resolution = (ledc_timer_bit_t)kResolution;
    timer.freq_hz = kFreqHz;
    timer.clk_cfg = LEDC_AUTO_CLK;
    err = ledc_timer_config(&timer);
    if (err != ESP_OK)
    {
        ESP_LOGW(TAG, "ledc_timer_config failed: %s", esp_err_to_name(err));
        return err;
    }

    // Channel
    ledc_channel_config_t ch = {};
    ch.gpio_num = PIN_SERVO;
    ch.speed_mode = kMode;
    ch.channel = kChannel;
    ch.intr_type = LEDC_INTR_DISABLE;
    ch.timer_sel = kTimer;
    ch.duty = 0;
    ch.hpoint = 0;
    err = ledc_channel_config(&ch);
    if (err != ESP_OK)
    {
        ESP_LOGW(TAG, "ledc_channel_config failed: %s", esp_err_to_name(err));
        return err;
    }

    // Optional: center on init
    err = servo_set_angle(90.0f);
    if (err == ESP_OK)
    {
        ESP_LOGI(TAG, "Servo initialized on GPIO %d (channel %d), centered to 90°",
                 (int)PIN_SERVO, (int)kChannel);
    }
    return err;
}

esp_err_t servo_set_pulse_us(unsigned int pulse_us)
{
    if (pulse_us < (unsigned)kMinUs)
        pulse_us = kMinUs;
    if (pulse_us > (unsigned)kMaxUs)
        pulse_us = kMaxUs;

    const uint32_t duty = us_to_duty(pulse_us);

    esp_err_t err = ledc_set_duty(kMode, kChannel, duty);
    if (err != ESP_OK)
    {
        ESP_LOGW(TAG, "ledc_set_duty failed: %s", esp_err_to_name(err));
        return err;
    }
    err = ledc_update_duty(kMode, kChannel);
    if (err != ESP_OK)
    {
        ESP_LOGW(TAG, "ledc_update_duty failed: %s", esp_err_to_name(err));
        return err;
    }

    ESP_LOGI(TAG, "Pulse set: %u us (duty=%u)", pulse_us, (unsigned)duty);
    return ESP_OK;
}

esp_err_t servo_set_angle(float degrees)
{
    if (degrees < 0.0f)
        degrees = 0.0f;
    if (degrees > 180.0f)
        degrees = 180.0f;

    const uint32_t pulse = (uint32_t)(kMinUs + (float)(kMaxUs - kMinUs) * (degrees / 180.0f));
    return servo_set_pulse_us(pulse);
}

esp_err_t open_servo(void)
{
    esp_err_t success = servo_set_angle(90);
    return success;
}

esp_err_t close_servo(void)
{
    esp_err_t success = servo_set_angle(0);
    return success;
}