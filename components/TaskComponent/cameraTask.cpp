#include "cameraTask.hpp"
#include "esp_log.h"
#include "mbedtls/base64.h"

#include "Client.hpp"
#include "communicationTask.hpp"
#include <driver/gpio.h>
#include <esp_idf_lib_helpers.h>
#include <esp_timer.h>
#include <ets_sys.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// For SD Card
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "driver/sdspi_host.h"
#include "driver/spi_common.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_defs.h"
#include "driver/sdmmc_host.h"
#include <dirent.h>

// For Clock
#include <time.h>
#include <sys/time.h>
#include "esp_sntp.h"

using namespace Zotbins;

static const char *name = "cameraTask";
static const int priority = 1;
static const uint32_t stackSize = 4096;

#define ESP_INR_FLAG_DEFAULT 0

// camera script
#include "esp_camera.h"
#include "esp_event.h"
#include "esp_http_server.h"
#include "esp_netif.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include <esp_log.h>
#include <esp_sleep.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <rom/ets_sys.h>
#include <string.h>
#include "zlib.h"
#include <stdlib.h>
#include <stdio.h>


#include "esp_now.h"


// FreeRTOS headers
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"

#define CAMERA_MODEL_AI_THINKER

// Board BOARD_ESP32CAM_AITHINKER
#ifdef CAMERA_MODEL_AI_THINKER
// ESP32Cam (AiThinker) PIN Map
#define CAM_PIN_PWDN 32
#define CAM_PIN_RESET -1
#define CAM_PIN_XCLK 0
#define CAM_PIN_SIOD 26
#define CAM_PIN_SIOC 27

#define CAM_PIN_D7 35
#define CAM_PIN_D6 34
#define CAM_PIN_D5 39
#define CAM_PIN_D4 36
#define CAM_PIN_D3 21
#define CAM_PIN_D2 19
#define CAM_PIN_D1 18
#define CAM_PIN_D0 5
#define CAM_PIN_VSYNC 25
#define CAM_PIN_HREF 23
#define CAM_PIN_PCLK 22
#endif

#define PART_BOUNDARY "123456789000000000000987654321"

static const char *TAG = "example:take_picture";
static const char *_STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char *_STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char *_STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

static EventGroupHandle_t s_wifi_event_group;
static esp_ip4_addr_t s_ip_addr;
static TaskHandle_t xTaskToNotify = NULL;
static const int core = 1;

#define WIFI_CONNECTED_BIT BIT0

static camera_config_t camera_config = {
    // CAM_PIN_PWDN
    // .pin_reset = CAM_PIN_RESET,
    .pin_pwdn = CAM_PIN_PWDN,
    .pin_reset = CAM_PIN_RESET,
    .pin_xclk = CAM_PIN_XCLK,
    .pin_sccb_sda = CAM_PIN_SIOD,
    .pin_sccb_scl = CAM_PIN_SIOC,

    .pin_d7 = CAM_PIN_D7,
    .pin_d6 = CAM_PIN_D6,
    .pin_d5 = CAM_PIN_D5,
    .pin_d4 = CAM_PIN_D4,
    .pin_d3 = CAM_PIN_D3,
    .pin_d2 = CAM_PIN_D2,
    .pin_d1 = CAM_PIN_D1,
    .pin_d0 = CAM_PIN_D0,
    .pin_vsync = CAM_PIN_VSYNC,
    .pin_href = CAM_PIN_HREF,
    .pin_pclk = CAM_PIN_PCLK,

    // XCLK 20MHz or 10MHz for OV2640 double FPS (Experimental)
    .xclk_freq_hz = 20000000,
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,

    .pixel_format = PIXFORMAT_JPEG,
    .frame_size = FRAMESIZE_VGA, // UXGA, VGA
    .jpeg_quality = 64,
    .fb_count = 2,
    .fb_location = CAMERA_FB_IN_PSRAM,// CAMERA_FB_IN_PSRAM,
    .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
};

const gpio_num_t flashPIN = GPIO_NUM_4;
TaskHandle_t camera_capture = NULL;
TaskHandle_t camera_countdown = NULL;

#define ESPNOW_MAXDELAY 512
#define CONFIG_ESPNOW_CHANNEL 1
#define ESPNOW_WIFI_MODE WIFI_MODE_STA
#define ESPNOW_WIFI_IF WIFI_IF_STA

// Fallback definition for MACSTR if not provided by esp_mac.h
#ifndef MACSTR
#define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"
#endif

static QueueHandle_t s_espnow_queue = NULL;
static uint8_t s_broadcast_mac[ESP_NOW_ETH_ALEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// Converts the Image buffer to a string
void buffer_to_string(uint8_t *buffer, size_t buffer_length, char *output, size_t output_size)
{
    size_t pos = 0;
    for (size_t i = 0; i < buffer_length; i++)
    {
        if (i > 0)
        {
            pos += snprintf(output + pos, output_size - pos, ",");
        }
        pos += snprintf(output + pos, output_size - pos, "%u", buffer[i]);
    }
}

// Initializes the Camera
static esp_err_t init_camera(void)
{
    // initialize the camera
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Camera Init Failed");
        return err;
    }
    ESP_LOGI(TAG, "Camera Init Worked");
    return ESP_OK;
}

// Initialize the SD Card
#define MOUNT_POINT "/sdcard"
esp_err_t init_sd_card() {
    esp_err_t ret;

    // Initialize SD card host (use SDMMC interface)
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    host.max_freq_khz = 200; // Reduce frequency to 400 kHz (try lower if needed)

    // Configure the slot with default values
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();

    // Set SDMMC slot configuration (optional adjustments if needed)
    slot_config.width = 1; // 1-line mode (can be 4-line for higher speeds if supported)
    slot_config.cd = SDMMC_SLOT_NO_CD;
    slot_config.wp = SDMMC_SLOT_NO_WP;

    // Configure the filesystem mount
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };

    // Pointer to SD card information
    sdmmc_card_t *card;

    // Mount the SD card
    ret = esp_vfs_fat_sdmmc_mount(MOUNT_POINT, &host, &slot_config, &mount_config, &card);
    if (ret != ESP_OK) {
        ESP_LOGE("SD", "Failed to mount SD card. Error code: 0x%x (%s)", ret, esp_err_to_name(ret));
        if (ret == ESP_ERR_TIMEOUT) {
            ESP_LOGE("SD", "This may be due to signal integrity issues or incompatible SD card.");
        } else if (ret == ESP_FAIL) {
            ESP_LOGE("SD", "SD card mount failed. Check card formatting and connections.");
        }
        return ret;
    }

    // Print SD card information if mounted successfully
    ESP_LOGI("SD", "SD card mounted successfully!");
    sdmmc_card_print_info(stdout, card);

    return ESP_OK;
}

// Reads from the text files in the sd card
void read_text_file(const char *path) {
    FILE *file = fopen(path, "r");
    if (!file) {
        ESP_LOGE(TAG, "Failed to open file: %s", path);
        return;
    }

    ESP_LOGI(TAG, "Reading file: %s", path);
    char line[256]; // Buffer to store lines from the file
    while (fgets(line, sizeof(line), file)) {
        printf("%s", line); // Print each line
    }

    fclose(file);
    ESP_LOGI(TAG, "Finished reading: %s", path);
}

// Iterates through the SD card and publishes to mqtt server
void publishSD(const char *dir_path) {
    DIR *dir = opendir(dir_path);
    if (!dir) {
        ESP_LOGE(TAG, "Failed to open directory: %s", dir_path);
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // Construct full file path
        char file_path[1024];
        snprintf(file_path, sizeof(file_path), "%s/%s", dir_path, entry->d_name);
        read_text_file(file_path);
    }

    closedir(dir);
}

int zlib_compress(const uint8_t *input, size_t input_size,
                  uint8_t **output, size_t *output_size)
{
    if (!input || input_size == 0 || !output || !output_size) {
        return -1; // Invalid arguments
    }

    // Estimate maximum size of compressed data
    uLong bound = compressBound(input_size);

    // Allocate memory for compressed data
    *output = (uint8_t *)malloc(bound);
    if (*output == NULL) {
        return -2; // Memory allocation failed
    }

    uLongf dest_len = bound;
    int res = compress2(*output, &dest_len, input, input_size, Z_BEST_COMPRESSION);
    if (res != Z_OK) {
        free(*output);
        *output = NULL;
        *output_size = 0;
        return -3; // Compression failed
    }

    *output_size = dest_len;
    return 0; // Success
}


CameraTask::CameraTask(QueueHandle_t &messageQueue)
    : Task(name, priority, stackSize), mMessageQueue(messageQueue)
{
}

void CameraTask::start()
{
    // xTaskCreatePinnedToCore(taskFunction, mName, mStackSize, this, mPriority, &xTaskToNotify, core);
    // TODO: for some stupid reason core 0 works for this, we need an explanation!!!
    // xTaskCreatePinnedToCore(taskFunction, mName, mStackSize, this, mPriority, &xTaskToNotify, 0);
    xTaskCreatePinnedToCore(taskFunction, mName, 65536, this, mPriority, &xTaskToNotify, 0);
}

void CameraTask::taskFunction(void *task)
{
    CameraTask *cameraTask = static_cast<CameraTask *>(task);
    cameraTask->setup();
    cameraTask->loop();
}

void CameraTask::setup()
{
}


void CameraTask::loop()
{

    while(1){
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        CommunicationTask::sendMessage("hello this is working properly");
    }







    

    // Client::clientPublish("cameraImage", (void*)"fsdjojsdg");
    

    // char *text = "255,216,255,224,0,16,74,70,73,70,0,1,1,1,0,0,0,0,0,0,255,219,0,67,0,12,8,9,11,9,8,12,11,10,11,14,13,12,14,18,30,20,18,17,17,18,37,26,28,22,30,44,38,46,45,43,38,42,41,48,54,69,59,48,51,65,52,41,42,60,82,61,65,71,74,77,78,77,47,58,85,91,84,75,90,69,76,77,74,255,219,0,67,1,13,14,14,18,16,18,35,20,20,35,74,50,42,50,74,74,74,74,74,74,74,74,74,74,74,74,74,74,74,74,74,74,74,74,74,74,74,74,74,74,74,74,74,74,74,74,74,74,74,74,74,74,74,74,74,74,74,74,74,74,74,74,74,74,255,196,0,31,0,0,1,5,1,1,1,1,1,1,0,0,0,0,0,0,0,0,1,2,3,4,5,6,7,8,9,10,11,255,196,0,181,16,0,2,1,3,3,2,4,3,5,5,4,4,0,0,1,125,1,2,3,0,4,17,5,18,33,49,65,6,19,81,97,7,34,113,20,50,129,145,161,8,35,66,177,193,21,82,209,240,36,51,98,114,130,9,10,22,23,24,25,26,37,38,39,40,41,42,52,53,54,55,56,57,58,67,68,69,70,71,72,73,74,83,84,85,86,87,88,89,90,99,100,101,102,103,104,105,106,115,116,117,118,119,120,121,122,131,132,133,134,135,136,137,138,146,147,148,149,150,151,152,153,154,162,163,164,165,166,167,168,169,170,178,179,180,181,182,183,184,185,186,194,195,196,197,198,199,200,201,202,210,211,212,213,214,215,216,217,218,225,226,227,228,229,230,231,232,233,234,241,242,243,244,245,246,247,248,249,250,255,196,0,31,1,0,3,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,1,2,3,4,5,6,7,8,9,10,11,255,196,0,181,17,0,2,1,2,4,4,3,4,7,5,4,4,0,1,2,119,0,1,2,3,17,4,5,33,49,6,18,65,81,7,97,113,19,34,50,129,8,20,66,145,161,177,193,9,35,51,82,240,21,98,114,209,10,22,36,52,225,37,241,23,24,25,26,38,39,40,41,42,53,54,55,56,57,58,67,68,69,70,71,72,73,74,83,84,85,86,87,88,89,90,99,100,101,102,103,104,105,106,115,116,117,118,119,120,121,122,130,131,132,133,134,135,136,137,138,146,147,148,149,150,151,152,153,154,162,163,164,165,166,167,168,169,170,178,179,180,181,182,183,184,185,186,194,195,196,197,198,199,200,201,202,210,211,212,213,214,215,216,217,218,226,227,228,229,230,231,232,233,234,242,243,244,245,246,247,248,249,250,255,192,0,17,8,0,240,1,64,3,1,33,0,2,17,1,3,17,1,255,218,0,12,3,1,0,2,17,3,17,0,63,0,225,35,76,145,235,159,74,111,27,55,252,220,253,220,210,87,234,33,201,156,131,193,221,129,197,48,54,227,199,39,25,226,167,155,82,45,97,195,31,83,159,202,162,83,132,231,147,252,170,244,52,108,149,183,7,199,106,77,195,39,167,173,79,34,232,76,23,97,235,34,249,103,119,168,251,189,104,99,242,141,165,191,46,106,29,238,36,172,195,4,48,244,247,163,104,25,31,123,142,107,94,107,138,218,232,69,34,175,149,142,77,44,158,157,241,67,189,134,214,131,119,124,223,47,62,224,82,70,193,126,241,237,82,82,86,21,71,28,122,247,164,39,133,232,1,167,206,59,15,254,33,199,166,51,222,163,81,128,113,216,81,5,109,65,142,32,96,224,156,103,173,53,177,199,83,239,78,236,95,16,236,252,202,5,53,72,220,74,130,5,18,109,232,59,10,23,228,63,54,114,120,166,99,3,27,89,177,234,106,111,166,166,99,177,251,197,237,210,154,190,152,205,87,49,160,127,203,39,250,138,105,25,84,193,205,15,98,71,255,0,203,69,7,219,138,106,28,110,30,217,169,123,12,63,229,155,125,69,55,35,96,254,116,250,8,127,89,20,128,112,113,138,96,254,33,223,20,249,135,112,80,190,81,231,191,165,53,143,202,162,171,155,168,199,1,251,193,237,138,108,71,140,253,105,61,68,94,28,33,232,48,119,84,100,99,111,247,215,129,208,86,107,204,209,232,56,237,243,70,238,184,200,237,154,23,229,219,211,110,222,224,82,228,33,177,70,114,202,120,230,163,251,203,242,254,89,170,113,229,220,90,92,121,79,159,63,118,145,182,29,199,171,17,215,52,174,222,195,230,182,195,203,99,118,220,42,110,6,163,199,31,222,249,49,242,253,106,122,139,152,158,63,154,124,125,231,35,28,210,168,219,128,188,231,142,105,236,199,113,155,67,69,199,220,228,242,122,82,200,139,179,112,220,163,216,211,82,11,16,148,62,110,120,28,242,59,82,39,220,199,183,122,185,10,225,201,67,142,41,204,48,50,14,1,236,41,114,164,198,183,14,55,40,37,131,113,212,231,181,55,147,27,99,114,252,190,180,129,142,114,118,190,14,57,233,76,219,189,6,223,155,241,165,96,184,167,253,106,145,237,205,34,2,185,108,113,86,210,216,1,83,247,109,128,122,246,63,90,107,252,173,24,172,194,232,87,255,0,88,187,186,119,163,110,57,4,96,14,41,45,5,113,191,242,204,241,220,115,71,77,167,218,172,119,208,95,227,7,233,72,0,222,195,28,255,0,58,45,98,46,3,253,75,125,69,35,30,7,81,138,118,212,112,220,114,103,120,227,210,152,6,115,244,165,97,183,102,32,95,221,28,115,200,164,199,221,7,52,236,43,143,7,247,170,61,169,131,215,245,245,162,193,98,224,93,196,142,126,106,111,155,242,169,45,193,161,235,160,110,11,251,194,10,142,191,48,246,166,156,41,198,118,251,84,94,195,108,147,111,222,141,66,238,207,83,186,144,252,203,156,99,241,167,118,33,51,243,115,252,93,40,12,119,159,156,238,199,167,52,253,11,146,210,226,200,193,84,237,25,57,239,248,210,55,200,19,159,225,228,209,177,158,194,164,164,79,242,191,32,14,156,82,41,207,112,202,7,126,244,165,168,237,239,18,110,67,247,95,99,251,115,77,45,243,245,230,161,169,92,123,14,198,249,48,118,224,245,205,48,148,218,48,71,202,42,239,168,8,74,128,113,144,51,211,25,164,39,231,199,109,185,253,106,173,125,202,243,31,140,178,227,190,42,16,87,30,220,244,161,14,232,126,252,197,211,239,98,142,118,3,129,184,117,231,21,54,50,210,226,127,203,69,227,142,41,189,9,250,82,47,81,57,104,15,205,233,147,70,122,31,144,96,103,138,187,43,104,47,33,196,126,241,62,108,99,20,208,125,121,226,163,80,118,16,127,169,63,239,10,58,227,3,35,25,170,179,0,220,30,68,219,140,80,175,243,227,229,227,245,169,93,152,13,231,202,110,7,106,86,251,163,154,166,198,183,15,249,106,191,133,49,48,122,142,212,239,160,49,121,242,219,253,234,79,225,5,122,80,75,216,113,32,76,158,188,119,168,211,190,78,120,162,253,74,131,47,125,229,219,158,114,58,143,173,7,145,188,145,207,63,157,23,75,97,106,5,242,233,191,145,145,198,41,34,218,24,227,143,151,35,63,90,54,142,133,109,160,49,254,53,39,46,255,0,149,52,244,18,118,254,245,40,124,58,147,168,229,221,188,18,156,99,29,41,57,62,163,3,243,168,106,218,128,6,249,49,220,118,29,41,100,254,19,215,156,99,166,106,250,14,193,159,154,53,63,135,181,55,32,144,59,227,154,168,177,10,167,17,158,221,179,235,78,225,121,97,184,246,59,170,57,154,149,135,184,140,255,0,62,56,193,197,46,255,0,238,140,122,26,22,215,16,47,9,128,127,250,212,140,188,12,245,170,243,11,88,65,233,219,138,79,249,232,167,223,191,74,106,200,55,144,142,255,0,187,251,189,14,77,5,113,176,12,116,169,231,176,236,133,60,76,131,216,116,166,149,13,33,81,247,59,103,173,88,8,24,50,55,167,7,129,73,130,170,20,246,253,106,110,33,122,203,143,165,34,146,50,184,247,160,30,186,0,63,187,60,0,9,166,183,56,39,174,49,69,196,197,4,249,160,114,113,138,98,244,43,140,113,235,77,217,5,180,36,222,124,183,250,250,211,27,144,62,185,161,108,16,208,7,19,175,225,75,25,195,127,74,150,94,160,159,234,250,28,2,63,30,180,198,99,181,119,26,104,92,183,20,255,0,172,92,251,83,35,250,255,0,13,0,180,47,150,43,192,3,175,248,212,123,135,150,49,184,28,98,154,166,79,81,115,185,151,39,230,227,241,164,94,65,36,103,142,62,92,212,94,218,20,32,251,167,56,246,160,142,132,42,253,113,87,160,164,24,204,131,28,109,29,232,83,223,158,6,42,91,118,176,212,87,81,217,38,50,188,255,0,136,164,220,219,113,145,183,110,41,219,64,20,28,202,50,163,241,165,86,108,17,159,151,211,168,53,159,45,152,189,68,80,54,224,128,113,67,30,23,21,97,240,138,155,150,78,49,202,243,73,194,169,249,193,0,117,161,197,92,93,110,25,49,240,121,57,231,138,77,199,3,11,158,184,226,151,42,43,168,163,46,202,123,119,246,166,114,56,207,99,205,28,170,246,23,192,195,36,71,193,233,142,244,173,209,118,144,42,180,43,152,122,182,14,20,113,247,170,53,249,27,37,122,10,155,16,55,159,44,143,238,158,148,108,30,88,218,126,180,125,145,236,46,239,222,140,243,210,145,49,146,7,76,82,179,40,80,15,148,221,249,31,214,153,252,32,244,20,11,160,255,0,249,104,191,133,68,163,46,114,127,134,141,216,181,29,255,0,44,219,228,207,35,169,164,236,187,184,31,74,177,92,119,30,106,252,220,241,198,41,145,50,145,219,167,74,123,162,175,112,207,238,216,158,187,169,175,192,92,169,169,182,161,114,79,249,106,61,70,42,36,250,240,5,1,204,95,14,118,255,0,22,61,189,105,27,238,242,128,128,189,74,230,153,67,147,247,78,167,183,31,79,165,50,3,129,130,113,199,61,134,42,87,113,59,116,20,49,17,127,113,114,62,148,215,228,96,99,165,34,53,23,112,47,150,221,207,56,110,212,245,60,149,85,63,54,91,138,169,110,85,152,195,158,152,57,95,122,83,243,40,249,62,188,209,205,174,130,81,2,15,154,57,37,143,171,113,77,94,131,130,63,10,119,208,118,67,114,62,99,130,48,125,104,207,201,141,172,62,180,227,181,216,49,216,27,215,230,228,116,164,4,231,129,142,58,245,168,91,147,186,12,183,150,120,33,137,231,138,9,202,134,6,171,168,210,19,31,190,95,238,241,154,98,175,205,207,167,20,115,189,133,123,146,116,141,176,57,226,152,221,22,132,30,99,215,239,143,147,30,188,210,242,115,253,220,84,236,39,184,207,224,99,201,199,60,83,88,1,143,169,166,62,163,177,153,87,61,177,77,92,242,0,165,175,82,150,161,255,0,44,137,3,161,20,214,36,168,60,228,213,216,37,160,225,196,160,251,10,106,253,227,140,30,41,2,19,159,45,184,43,200,246,165,102,5,20,209,46,132,117,16,125,244,207,94,41,187,137,200,35,168,61,5,27,104,49,71,48,158,58,26,87,57,218,61,233,21,96,193,55,28,123,119,166,166,238,235,206,40,114,17,127,129,141,229,182,3,81,252,162,32,14,56,29,73,53,92,218,4,152,225,247,192,221,180,251,26,68,35,140,246,169,79,160,114,221,4,108,216,61,114,63,187,218,147,105,59,122,230,154,81,128,218,176,225,243,127,16,245,164,246,97,142,40,231,177,13,220,51,129,248,254,180,230,218,99,7,238,237,254,44,212,183,212,29,211,2,254,167,191,95,90,97,24,24,108,39,160,205,61,22,133,47,138,226,108,38,60,241,218,140,227,0,140,28,84,234,195,70,59,63,188,24,24,207,60,212,75,159,152,118,252,170,213,172,26,36,56,124,137,151,235,145,65,27,64,31,197,80,159,188,36,31,119,234,72,166,168,10,91,191,203,214,180,210,225,16,76,44,25,227,4,138,70,251,131,233,218,147,9,14,7,46,63,218,199,56,160,22,253,41,63,136,18,5,206,215,206,56,63,227,72,78,87,118,220,241,235,78,214,43,113,7,18,47,252,6,147,28,243,131,199,106,157,66,201,11,242,249,103,234,41,167,230,11,131,138,164,251,135,168,160,143,49,125,241,222,152,153,220,70,59,117,163,98,65,64,242,219,161,201,2,130,159,115,109,45,65,90,224,63,214,32,39,251,180,115,147,219,143,74,58,148,38,127,116,125,207,248,211,127,133,106,144,144,239,249,104,164,143,74,68,60,126,20,75,93,132,238,92,12,84,26,115,126,190,181,50,107,160,73,69,13,117,1,186,115,73,207,157,133,46,56,207,94,40,137,49,184,156,224,253,122,211,155,59,10,255,0,17,200,6,147,87,52,178,28,195,231,92,119,232,41,189,251,99,4,154,94,161,176,159,55,63,48,0,241,210,142,191,221,227,189,104,154,216,158,163,131,16,255,0,95,122,140,112,221,57,199,235,80,162,239,169,40,23,59,89,71,191,34,131,192,92,124,192,10,58,232,93,253,225,48,60,206,153,233,71,94,61,7,21,68,238,10,8,70,198,58,208,199,17,252,198,147,236,139,94,96,164,239,92,123,80,59,143,110,244,73,15,160,213,220,21,176,127,138,145,193,226,146,100,105,113,205,203,122,30,63,42,68,238,125,169,140,104,255,0,82,255,0,95,241,165,194,148,94,121,21,77,216,119,236,56,124,173,206,15,78,106,53,207,64,119,46,57,247,162,224,56,255,0,171,111,173,49,190,85,28,30,149,26,108,33,199,137,70,48,122,118,166,238,59,206,51,140,113,154,184,235,160,88,58,163,115,220,82,21,253,218,210,150,130,66,143,245,138,48,59,116,164,78,75,28,246,167,29,202,21,71,238,91,39,191,79,206,154,195,229,20,182,11,135,62,100,120,246,166,142,73,244,197,27,11,93,203,239,159,44,237,228,250,211,29,190,97,181,219,111,110,217,163,144,109,11,177,25,149,184,232,51,158,185,197,40,63,47,7,255,0,173,82,182,179,16,222,62,110,122,26,119,251,47,157,163,28,26,190,94,163,28,192,172,159,123,60,227,173,51,248,134,224,50,83,230,238,42,100,180,184,75,123,138,27,228,125,142,89,115,235,214,163,97,242,47,204,127,198,141,131,152,127,241,115,70,220,111,254,47,161,235,85,184,246,25,252,45,236,216,163,230,192,7,57,3,189,75,78,228,232,56,18,210,47,247,15,52,212,249,73,6,148,157,244,24,21,45,19,124,221,27,167,122,63,229,158,57,197,12,46,56,140,202,166,152,5,50,181,19,119,223,3,160,35,250,210,30,131,175,231,76,145,195,253,98,228,250,82,6,37,219,142,222,180,194,194,0,124,179,252,191,58,86,57,97,234,125,234,62,36,75,209,129,199,152,63,10,68,61,242,122,81,203,238,149,113,55,31,39,25,227,165,53,178,66,231,173,85,129,52,60,175,41,142,27,138,98,103,45,253,77,43,232,38,42,115,19,102,154,255,0,113,72,165,203,160,135,127,203,69,199,181,48,117,171,184,238,46,127,114,126,191,227,72,223,117,105,19,176,159,242,217,113,237,72,157,121,244,164,106,164,104,74,216,135,229,126,7,173,50,77,165,183,34,228,14,244,172,204,215,112,144,109,147,3,7,105,0,158,148,153,12,79,191,234,41,249,149,208,118,65,92,29,184,250,83,112,48,159,55,108,115,222,174,77,34,54,66,255,0,203,81,239,77,133,194,224,231,182,121,20,155,186,178,46,247,136,47,220,234,88,143,90,118,88,160,7,7,31,236,142,43,45,222,161,230,38,126,101,200,61,185,197,36,103,119,4,124,220,213,177,233,109,4,200,10,64,232,184,165,94,72,61,123,254,20,239,97,11,247,93,23,118,113,199,20,212,193,29,71,212,243,72,87,98,231,168,7,57,61,77,52,109,85,28,252,180,134,135,14,171,198,227,199,169,164,0,103,239,103,235,84,54,5,191,118,122,123,122,212,103,238,39,229,85,109,5,228,63,0,178,21,246,166,134,251,223,54,14,42,86,160,8,15,150,222,228,82,48,24,90,119,13,5,254,53,245,226,142,50,77,45,82,36,104,251,164,251,255,0,141,13,198,223,210,161,134,195,148,143,53,56,244,166,109,231,240,197,86,197,116,2,63,116,249,245,20,223,225,92,81,109,9,231,30,71,239";

    // size_t text_size = strlen(text);
    // uint8_t *compressed = NULL;
    // size_t compressed_size = 0;
    // printf("Text size: %d\n", text_size);

    // if (zlib_compress((const uint8_t *)text, text_size, &compressed, &compressed_size) == 0 ){
    //     printf("Compressed size: %d\n", compressed_size);
    //     Client::clientPublishStr(&compressed_size);
    //     Client::clientPublishStr(&text_size);
    //     Client::clientPublishStr(compressed);
    //     free(compressed);
    // } else {
    //     printf("Compression failed.\n");
    // }
    

    // while(1){
    //     vTaskDelay(1000 / portTICK_PERIOD_MS);
    // }

    // Wait for Notification
    //ulTaskNotifyTake(pdTRUE, (TickType_t)portMAX_DELAY); 
    ESP_LOGI(TAG, "Hello from Camera Task");

    // Intialize SD Card
    // esp_err_t ret = init_sd_card();
    // if (ret != ESP_OK) {
    //     ESP_LOGE("SD", "SD card initialization failed");
    // }
    // else{
    //     ESP_LOGE("SD", "SD card initialized");
    // }

    // Intialize the Necessary GPIO 
    gpio_reset_pin(flashPIN);
    gpio_set_direction(flashPIN, GPIO_MODE_OUTPUT);
    gpio_set_pull_mode(flashPIN, GPIO_PULLDOWN_ONLY);
    gpio_set_level(flashPIN, 0);

    // Initialize NVS
    ESP_ERROR_CHECK(nvs_flash_init());
 
    // Initialize the camera
    if (ESP_OK != init_camera()){
        ESP_LOGE(TAG, "Camera Failed");
    }else{
        ESP_LOGI(TAG, "Camera started, getting sensor");
    }

    // Setup Camera 
    sensor_t *s = esp_camera_sensor_get();
    if (s != NULL){
        s->set_sharpness(s, 1);
        s->set_gain_ctrl(s, 1);
        s->set_whitebal(s, 1);
    }

    // Camera Functionality
    camera_fb_t *fb = NULL;
    int cnt = 0;
    
    while (1)
    {
        cnt++;
        if (cnt == 5)
        {
            // Convert to buffer to readable format
            fb = esp_camera_fb_get();
            size_t output_size = 262144; 
            char *output = (char *)malloc(output_size);
            buffer_to_string(fb->buf, fb->len, output, output_size);

            Client::clientPublish("camera", output);
                        
            free(output);
            

            // ESP_LOGE(TAG, "Helo");
            // char *compressed_data = NULL;
            // int compressed_size = 0;
            // printf("Compressed size: %d\n", compressed_size);

            // Call decompress function
            // int ret = decompress_data(compressed_data, compressed_size, &decompressed_data, &decompressed_size);
            // if (ret == Z_OK) {
            //     printf("Decompression successful\n");
            //     printf("Decompressed data: %s\n", decompressed_data);
            // } else {
            //     printf("Decompression failed with error: %d\n", ret);
            // }

            // Save Information to SD card
            // Open a file for writing (FILE_WRITE flag ensures data is written)
            // FILE *f = fopen("/sdcard/test.txt", "w");
            // if (f == NULL) {
            //     ESP_LOGE(TAG, "Failed to open file for writing");
            // }
            // else{
            //     ESP_LOGI(TAG, "File written successfully!");
            //     fprintf(f, compressed_data);
            // }


            // fclose(f);
            esp_camera_fb_return(fb);

            
            
            // xTaskToNotify = xTaskGetHandle("usageTask"); // servoTask");
            // xTaskNotifyGive(xTaskToNotify);
            // break;
        }
        vTaskDelay(35 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}
