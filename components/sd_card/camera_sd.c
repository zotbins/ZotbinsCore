#include "esp_camera.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"
#include <stdio.h>

static const char *TAG = "CAM_SD";

void camera_sd_init(void)
{
    // --- Camera config (ESP32-CAM) ---
    camera_config_t config = {
        .pin_pwdn = 32,
        .pin_reset = -1,
        .pin_xclk = 0,
        .pin_sccb_sda = 26,
        .pin_sccb_scl = 27,

        .pin_d7 = 35,
        .pin_d6 = 34,
        .pin_d5 = 39,
        .pin_d4 = 36,
        .pin_d3 = 21,
        .pin_d2 = 19,
        .pin_d1 = 18,
        .pin_d0 = 5,
        .pin_vsync = 25,
        .pin_href = 23,
        .pin_pclk = 22,

        .xclk_freq_hz = 20000000,
        .pixel_format = PIXFORMAT_JPEG,
        .frame_size = FRAMESIZE_VGA,
        .jpeg_quality = 12,
        .fb_count = 1
    };

    if (esp_camera_init(&config) != ESP_OK) {
        ESP_LOGE(TAG, "Camera init failed");
        return;
    }

    ESP_LOGI(TAG, "Camera initialized");

    // --- SD card init ---
    sdmmc_card_t *card;
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
    slot_config.width = 1;

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5
    };

    if (esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot_config, &mount_config, &card) != ESP_OK) {
        ESP_LOGE(TAG, "SD mount failed");
        return;
    }

    ESP_LOGI(TAG, "SD card mounted");
}

void capture_and_save_image(const char *filename)
{
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        ESP_LOGE(TAG, "Capture failed");
        return;
    }

    FILE *f = fopen(filename, "w");
    if (!f) {
        ESP_LOGE(TAG, "File open failed");
        esp_camera_fb_return(fb);
        return;
    }

    fwrite(fb->buf, 1, fb->len, f);
    fclose(f);

    ESP_LOGI(TAG, "Saved: %s", filename);

    esp_camera_fb_return(fb);
}