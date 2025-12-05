#include <stdio.h>
#include "esp_system.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_camera.h"
#include "driver/gpio.h"

static const char *TAG = "cam";

// WROVER-KIT default camera pinout
#define CAM_PIN_PWDN    -1
#define CAM_PIN_RESET   -1
#define CAM_PIN_XCLK    21
#define CAM_PIN_SIOD    26
#define CAM_PIN_SIOC    27

#define CAM_PIN_D7      35
#define CAM_PIN_D6      34
#define CAM_PIN_D5      39
#define CAM_PIN_D4      36
#define CAM_PIN_D3      19
#define CAM_PIN_D2      18
#define CAM_PIN_D1       5
#define CAM_PIN_D0       4

#define CAM_PIN_VSYNC   25
#define CAM_PIN_HREF    23
#define CAM_PIN_PCLK    22

// Initialize camera
static esp_err_t init_camera(void)
{
    // Enable internal pull-ups for SIOD/SIOC
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << CAM_PIN_SIOD) | (1ULL << CAM_PIN_SIOC),
        .mode = GPIO_MODE_INPUT_OUTPUT_OD, 
        .pull_up_en = GPIO_PULLUP_ENABLE,  
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);

    camera_config_t config = {
        .pin_pwdn       = CAM_PIN_PWDN,
        .pin_reset      = CAM_PIN_RESET,
        .pin_xclk       = CAM_PIN_XCLK,
        .pin_sccb_sda   = CAM_PIN_SIOD,
        .pin_sccb_scl   = CAM_PIN_SIOC,

        .pin_d7         = CAM_PIN_D7,
        .pin_d6         = CAM_PIN_D6,
        .pin_d5         = CAM_PIN_D5,
        .pin_d4         = CAM_PIN_D4,
        .pin_d3         = CAM_PIN_D3,
        .pin_d2         = CAM_PIN_D2,
        .pin_d1         = CAM_PIN_D1,
        .pin_d0         = CAM_PIN_D0,

        .pin_vsync      = CAM_PIN_VSYNC,
        .pin_href       = CAM_PIN_HREF,
        .pin_pclk       = CAM_PIN_PCLK,

        .xclk_freq_hz   = 8000000, 
        .ledc_timer     = LEDC_TIMER_0,
        .ledc_channel   = LEDC_CHANNEL_0,

        .pixel_format   = PIXFORMAT_JPEG,
        .frame_size     = FRAMESIZE_SVGA,
        .jpeg_quality   = 12,
        .fb_count       = 2,
        .fb_location    = CAMERA_FB_IN_PSRAM,
        .grab_mode      = CAMERA_GRAB_WHEN_EMPTY,
    };

    ESP_LOGI(TAG, "Initializing camera...");
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera init failed: 0x%x", err);
        return err;
    }

    sensor_t *s = esp_camera_sensor_get();
    if (!s) {
        ESP_LOGE(TAG, "Failed to get camera sensor");
        return ESP_FAIL;
    }

    s->set_quality(s, 12);
    s->set_framesize(s, FRAMESIZE_SVGA);

    vTaskDelay(pdMS_TO_TICKS(500));
    return ESP_OK;
}


static void camera_task(void *pvParameters)
{
    uint32_t frame_count = 0;

    vTaskDelay(pdMS_TO_TICKS(2000));
    ESP_LOGI(TAG, "Starting frame capture...");

    while (1) {
        camera_fb_t *fb = esp_camera_fb_get();
        if (!fb) {
            ESP_LOGE(TAG, "Frame capture failed");
            vTaskDelay(pdMS_TO_TICKS(2000));
            continue;
        }

        frame_count++;

        ESP_LOGI(TAG, "Frame #%lu: %d bytes, %dx%d",
                 frame_count, fb->len, fb->width, fb->height);

        esp_camera_fb_return(fb);
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "=== ESP32 Camera Application ===");
    ESP_LOGI(TAG, "Free heap: %lu bytes", esp_get_free_heap_size());

    vTaskDelay(pdMS_TO_TICKS(100));

    if (init_camera() != ESP_OK) {
        ESP_LOGE(TAG, "Camera initialization failed!");
        return;
    }

    ESP_LOGI(TAG, "Free heap after init: %lu bytes", esp_get_free_heap_size());

    xTaskCreate(camera_task, "camera_task", 4096, NULL, 5, NULL);
    ESP_LOGI(TAG, "Camera task created");
}
