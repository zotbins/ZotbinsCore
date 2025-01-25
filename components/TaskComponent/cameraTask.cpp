#include "cameraTask.hpp"
#include "esp_log.h"

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
    .frame_size = FRAMESIZE_QVGA,
    .jpeg_quality = 12,
    .fb_count = 2,
    .fb_location = CAMERA_FB_IN_PSRAM,
    .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
};

// Pin Declarations
const gpio_num_t flashPIN = GPIO_NUM_4;
const gpio_num_t inputPIN = GPIO_NUM_14;
const gpio_num_t ledPin1 = GPIO_NUM_1;
const gpio_num_t ledPin2 = GPIO_NUM_3;
TaskHandle_t camera_capture = NULL;
TaskHandle_t camera_countdown = NULL;

// Possible to get 3 LEDS if you solder an AND gate
bool takePicture = false;
bool allowPicture = true;

void vCountdown(void *pvParameters)
{
    // The LED System
    // takePicture = false;
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    gpio_set_level(ledPin1, 1);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    gpio_set_level(ledPin2, 1);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    // takePicture = true;
    xTaskNotifyGive(camera_capture); // capture camera parallel task

    gpio_set_level(flashPIN, 1);
    vTaskDelay(250 / portTICK_PERIOD_MS);
    gpio_set_level(flashPIN, 0);
    // vTaskDelete(camera_countdown);
}

// THIS ONLY WORKS IF ONLY Camera Task
void startSleep()
{
    esp_sleep_enable_ext0_wakeup(inputPIN, 1);
    esp_light_sleep_start();
}

void buffer_to_string(uint8_t *buffer, size_t buffer_length, char *output, size_t output_size)
{
    size_t pos = 0;
    //pos += snprintf(output + pos, output_size - pos, "[");
    for (size_t i = 0; i < buffer_length; i++)
    {
        if (i > 0)
        {
            pos += snprintf(output + pos, output_size - pos, ",");
        }
        pos += snprintf(output + pos, output_size - pos, "%u", buffer[i]);
    }
    //snprintf(output + pos, output_size - pos, "]");
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
    ESP_LOGE(TAG, "Camera Init Worked");
    return ESP_OK;
}

CameraTask::CameraTask(QueueHandle_t &messageQueue)
    : Task(name, priority, stackSize), mMessageQueue(messageQueue)
{
}

void CameraTask::start()
{
    xTaskCreatePinnedToCore(taskFunction, mName, mStackSize, this, mPriority, nullptr, 1);
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

//  xTaskCreatePinnedToCore(taskFunction, mName, mStackSize, this, mPriority, nullptr, 1);
    gpio_reset_pin(flashPIN);
    gpio_set_direction(flashPIN, GPIO_MODE_OUTPUT);
    gpio_set_pull_mode(flashPIN, GPIO_PULLDOWN_ONLY);
    gpio_set_level(flashPIN, 0);

    gpio_reset_pin(inputPIN);
    gpio_set_direction(inputPIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(inputPIN, GPIO_PULLDOWN_ONLY);
    gpio_set_level(inputPIN, 0);

    gpio_reset_pin(GPIO_NUM_0);
    gpio_set_direction(flashPIN, GPIO_MODE_OUTPUT);
    gpio_set_pull_mode(flashPIN, GPIO_PULLDOWN_ONLY);
    gpio_set_level(flashPIN, 0);

    // Initialize NVS
    ESP_ERROR_CHECK(nvs_flash_init());

    // Initialize the camera
    if (ESP_OK != init_camera())
    {
        Client::clientPublish("Camera Failed");
    }

    Client::clientPublish("Started Camera"); 

    sensor_t *s = esp_camera_sensor_get();
    if (s != NULL) {
        s->set_brightness(s, 1);
        s->set_contrast(s, 1);
        s->set_saturation(s, 2);
        s->set_sharpness(s, 1);
        s->set_gain_ctrl(s, 1);
        s->set_whitebal(s, 1);
        Client::clientPublish("Adjusted Camera");
    }

    
    camera_fb_t *fb = NULL;
    while(1){

        if(gpio_get_level(inputPIN) == 1){
            for(int i = 0; i < 30; i++){
                if(i == 29){
                    gpio_set_level(flashPIN, 1);
                }
            fb = esp_camera_fb_get();
            vTaskDelay(33 / portTICK_PERIOD_MS);
            esp_camera_fb_return(fb);
            }
            vTaskDelay(200 / portTICK_PERIOD_MS);
            gpio_set_level(flashPIN, 0);
            uint8_t *buffer = fb->buf;      
            size_t buffer_length = fb->len;
            size_t output_size = 65536; 
            char *output_string = (char *)malloc(output_size);
            buffer_to_string(buffer, buffer_length, output_string, output_size);
            Client::clientPublish(output_string);
            vTaskDelay(4500 / portTICK_PERIOD_MS);
        }
        vTaskDelay(33 / portTICK_PERIOD_MS);
    }

}
