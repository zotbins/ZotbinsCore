#include <stdio.h>
#include "esp_log.h"
#include "esp_err.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"

static const char *TAG = "SD_CAM";

void app_main(void)
{
    esp_err_t ret;

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };

    sdmmc_card_t *card;
    const char mount_point[] = "/sdcard";

    sdmmc_host_t host = SDMMC_HOST_DEFAULT();

    // ESP32-CAM often works more reliably in 1-bit mode
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
    slot_config.width = 1;

    ret = esp_vfs_fat_sdmmc_mount(mount_point, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount SD card (%s)", esp_err_to_name(ret));
        return;
    }

    ESP_LOGI(TAG, "SD card mounted");

    // Write file
    FILE *f = fopen("/sdcard/test.txt", "w");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return;
    }
    fprintf(f, "Hello from ESP32-CAM!\n");
    fclose(f);
    ESP_LOGI(TAG, "File written");

    // Read file
    f = fopen("/sdcard/test.txt", "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return;
    }

    char line[64];
    fgets(line, sizeof(line), f);
    fclose(f);

    ESP_LOGI(TAG, "Read: %s", line);

    // Unmount (optional)
    esp_vfs_fat_sdcard_unmount(mount_point, card);
    ESP_LOGI(TAG, "SD card unmounted");
}