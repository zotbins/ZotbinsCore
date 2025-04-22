#include "cameraTask.hpp"
#include "esp_log.h"
#include "mbedtls/base64.h"

#include "Client.hpp"
#include <driver/gpio.h>
#include <esp_idf_lib_helpers.h>
#include <esp_timer.h>
#include <ets_sys.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>


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
// #include "zlib.h"


// FreeRTOS headers
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"

// BOARD_ESP32CAM_AITHINKER

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
    .fb_count = 8,
    .fb_location = CAMERA_FB_IN_PSRAM,
    .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
};

const gpio_num_t flashPIN = GPIO_NUM_4;
TaskHandle_t camera_capture = NULL;
TaskHandle_t camera_countdown = NULL;

void buffer_to_string(uint8_t *buffer, size_t buffer_length, char *output, size_t output_size)
{
    size_t pos = 0;
    for (size_t i = 0; i < buffer_length; i++)
    {
        // vTaskDelay(2 / portTICK_PERIOD_MS);
        if (i > 0)
        {
            pos += snprintf(output + pos, output_size - pos, ",");
        }
        pos += snprintf(output + pos, output_size - pos, "%u", buffer[i]);
    }
}

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

CameraTask::CameraTask(QueueHandle_t &messageQueue)
    : Task(name, priority, stackSize), mMessageQueue(messageQueue)
{
}

void CameraTask::start()
{
    // xTaskCreatePinnedToCore(taskFunction, mName, mStackSize, this, mPriority, &xTaskToNotify, core);
    // TODO: for some stupid reason core 0 works for this, we need an explanation!!!
    xTaskCreatePinnedToCore(taskFunction, mName, mStackSize, this, mPriority, &xTaskToNotify, 0);
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
    ESP_LOGI(TAG, "Hello from Camera Task");
    // ulTaskNotifyTake(pdTRUE, (TickType_t)portMAX_DELAY);

    gpio_reset_pin(flashPIN);
    gpio_set_direction(flashPIN, GPIO_MODE_OUTPUT);
    gpio_set_pull_mode(flashPIN, GPIO_PULLDOWN_ONLY);
    gpio_set_level(flashPIN, 0);

    // Initialize NVS
    ESP_ERROR_CHECK(nvs_flash_init());
 
    // Initialize the camera
    if (ESP_OK != init_camera()){
        // Client::clientPublishStr("Camera Failed");
        ESP_LOGE(TAG, "Camera Failed");
    }else{
        // Client::clientPublishStr("Started Camera");
        ESP_LOGI(TAG, "Camera started, getting sensor");
    }

    sensor_t *s = esp_camera_sensor_get();
    if (s != NULL){
        s->set_sharpness(s, 1);
        s->set_gain_ctrl(s, 1);
        s->set_whitebal(s, 1);
    }

    ESP_LOGI(TAG, "Camera sensor gotted, loop start");

    camera_fb_t *fb = NULL;
    int cnt = 0;
    while (1)
    {
        cnt++;
        if (cnt == 5)
        {
            fb = esp_camera_fb_get();

            ESP_LOGI(TAG, "Camera fb get done, sending");

            size_t output_size = fb->len * 4;
            char *output = (char *)malloc(output_size);
            // needs time to allocate 
            vTaskDelay(2000 / portTICK_PERIOD_MS);
            buffer_to_string(fb->buf, fb->len, output, output_size);
            Client::clientPublish("camera", output);
            ESP_LOGI(TAG, "Camera published"); 

            // TODO: fix inconsistent bootloops
            esp_camera_fb_return(fb);
            // xTaskToNotify = xTaskGetHandle("usageTask"); // servoTask");
            // xTaskNotifyGive(xTaskToNotify);
            break;
        }
        vTaskDelay(35 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}
