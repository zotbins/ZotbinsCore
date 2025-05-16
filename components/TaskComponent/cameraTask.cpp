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
    .frame_size = FRAMESIZE_SVGA, // UXGA, VGA, SVGA(800x600)
    /* TODO: eventually we want to be able to send UXGA quality (1600x1200)
        however, MQTT doesn't allow sizes above a certain threshold which UXGA exceeds
        so we need to divert the package 
    */
    .jpeg_quality = 12,
    .fb_count = 8, //  change this for faster
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
        ESP_LOGE(name, "Camera Init Failed");
        return err;
    }
    ESP_LOGI(name, "Camera Init Worked");
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
        ESP_LOGE(name, "Failed to open file: %s", path);
        return;
    }

    ESP_LOGI(name, "Reading file: %s", path);
    char line[256]; // Buffer to store lines from the file
    while (fgets(line, sizeof(line), file)) {
        printf("%s", line); // Print each line
    }

    fclose(file);
    ESP_LOGI(name, "Finished reading: %s", path);
}

// Iterates through the SD card and publishes to mqtt server
void publishSD(const char *dir_path) {
    DIR *dir = opendir(dir_path);
    if (!dir) {
        ESP_LOGE(name, "Failed to open directory: %s", dir_path);
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

    uint8_t mac[6];  // Array to store the MAC address
    esp_wifi_get_mac(WIFI_IF_STA, mac);  // Get Wi-Fi station MAC address
    printf("ESP32 MAC Address: %02X:%02X:%02X:%02X:%02X:%02X\n",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);


    ESP_LOGE(name, "Task Function");
    xTaskCreatePinnedToCore(taskFunction, mName, 4096, this, mPriority, &xTaskToNotify, 0);
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


    uint8_t mac[6];  // Array to store the MAC address

    esp_wifi_get_mac(WIFI_IF_STA, mac);  // Get Wi-Fi station MAC address

    printf("ESP32 MAC Address: %02X:%02X:%02X:%02X:%02X:%02X\n",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);



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
        ESP_LOGE(name, "Camera Failed");
    }else{
        ESP_LOGI(name, "Camera started, getting sensor");
    }

    // Setup Camera 
    sensor_t *s = esp_camera_sensor_get();
    if (s != NULL) {
        // Image tuning
        s->set_sharpness(s, 1);         // Optional, 0–2
        s->set_brightness(s, 2);        // -2 to 2 (2 = brighter)
        s->set_contrast(s, 0);          // -2 to 2
        s->set_saturation(s, 0);        // -2 to 2

        // Gain and exposure
        s->set_gain_ctrl(s, 1);         // Enable auto gain
        s->set_exposure_ctrl(s, 1);     // Enable auto exposure
        s->set_gainceiling(s, (gainceiling_t)6); // Try 6 or 8 for more gain headroom

        // White balance
        s->set_whitebal(s, 1);          // Enable auto white balance
        s->set_awb_gain(s, 1);          // Enable auto white balance gain
    }

    // Camera Functionality
    camera_fb_t *fb = NULL;
    int cnt = 0;
    ESP_LOGE(name, "Camera Setup and Waiting!");
    ulTaskNotifyTake(pdTRUE, (TickType_t)portMAX_DELAY);
    ESP_LOGE(name, "Camera Loop Starting");
    while (1)
    {
        cnt++; // need to wait for something...? (green tint, it was initializing too quickly)
        fb = esp_camera_fb_get();
        uint64_t start = esp_timer_get_time();  

        if (cnt == 30)
        {
            size_t output_size = 128000; // 262144 for 800 x 600; 
            char *output = (char *)malloc(output_size);
            buffer_to_string(fb->buf, fb->len, output, output_size);
            //printf("%s\n", output);
            ESP_LOGE(name, "Taking Picture");
            vTaskDelay(3000 / portTICK_PERIOD_MS);
            free(output);
            xTaskToNotify = xTaskGetHandle("servoTask"); // servoTask");
            vTaskResume(xTaskToNotify);  
            ulTaskNotifyTake(pdTRUE, (TickType_t)portMAX_DELAY);   
            cnt = 0;
        }
        esp_camera_fb_return(fb);
        vTaskDelay(35 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}
