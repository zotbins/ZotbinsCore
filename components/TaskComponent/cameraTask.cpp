#include "cameraTask.hpp"
#include "esp_log.h"

#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_timer.h>
#include <ets_sys.h>
#include <esp_idf_lib_helpers.h>

using namespace Zotbins;

static const char *name = "cameraTask";
static const int priority = 1;
static const uint32_t stackSize = 4096;
static SemaphoreHandle_t cameraSem;


// camera script
#include "esp_camera.h"
#include "esp_event.h"
#include "esp_http_server.h"
#include "esp_netif.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <string.h>
#include <esp_system.h>
#include <esp_sleep.h>
#include <rom/ets_sys.h>


// FreeRTOS headers
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"

#define BOARD_ESP32CAM_AITHINKER

#ifdef BOARD_ESP32CAM_AITHINKER
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

typedef struct {
    gpio_num_t pin;
    int duration_ms;
} pwm_task_params_t;

void pwm(void *pvParameters){
    // pwm_task_params_t *params = (pwm_task_params_t *)pvParameters;
    // gpio_num_t pin = params->pin;
    // int duration_ms = params->duration_ms;

    // int current;

    // while (current > duration_ms) {
    //     gpio_set_level(pin, 1);
    //     vTaskDelay(100);
    //     gpio_set_level(pin, 0);
    //     vTaskDelay(pdMS_TO_TICKS(100));
    // }

    // gpio_set_level(pin, 0);
    // vTaskDelete(NULL);



}


bool allowPicture = true; 
bool takePicture = false;
void vCountdown(){    
    takePicture = false;
    //pwm(ledPin1);
    // pwm(ledPin2);
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    takePicture = true;
    gpio_set_level(flashPIN,1);
    vTaskDelay(250 / portTICK_PERIOD_MS);
    gpio_set_level(flashPIN,0);
    vTaskDelete(NULL);
}


void startSleep(){
    esp_sleep_enable_ext0_wakeup(inputPIN, 1); 
    esp_light_sleep_start();
    // while(1){
    //      if(gpio_get_level(inputPIN) == 1){
    //          break;
    //      }
    // }
    
}


void exportImage(int *buffer, size_t length){
    // FILE *file;
    // file = fopen(fname, "w");
    // fprintf(file, "{\n  \"numbers\": [");
    // for (size_t i = 0; i < length; ++i) {
    //     fprintf(file, "%u", *(buffer+i));
    //     if(i < (length - 1)){
    //         fprintf(file, ", ");
    //     }
    // }
    // fprintf(file, "]\n}");
    // fclose(file);
}

esp_err_t jpg_stream_httpd_handler(httpd_req_t *req)
{
    
    allowPicture = true;
    camera_fb_t *fb = NULL;
    esp_err_t res = ESP_OK;
    size_t _jpg_buf_len;
    uint8_t *_jpg_buf;
    char part_buf[128];
    static int64_t last_frame = 0;
    if (!last_frame)
    {
        last_frame = esp_timer_get_time();
    }

    res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
    if (res != ESP_OK)
    {
        return res;
    }

    
    // startSleep();
    while (true)
    {       
        
        fb = esp_camera_fb_get();

        if (!fb)
        {
            ESP_LOGE(TAG, "Camera capture failed");
            res = ESP_FAIL;
            break;
        }
        if (fb->format != PIXFORMAT_JPEG)
        {
            bool jpeg_converted = frame2jpg(fb, camera_config.jpeg_quality, &_jpg_buf, &_jpg_buf_len);
            if (!jpeg_converted)
            {
                ESP_LOGE(TAG, "JPEG compression failed");
                esp_camera_fb_return(fb);
                res = ESP_FAIL;
                break;
            }
        }
        else
        {
            _jpg_buf_len = fb->len;
            _jpg_buf = fb->buf;
        }     

        
        
        if(gpio_get_level(inputPIN) == 1 && allowPicture == true){
            allowPicture = false;
            // xTaskCreate(vCountdown, "Countdown", 2048, NULL, 1, NULL); // Picture Timer
        }
        
    
        if(takePicture == true){

            uint8_t *buffer = fb->buf;  // Pointer to the buffer
            size_t buffer_length = fb->len;  // Length of the buffer in bytes
            gpio_set_level(flashPIN, 0);

            ESP_LOGE(TAG, "Buffer Pointer: %p", buffer);
            ESP_LOGE(TAG, "Buffer Length: %u", buffer_length);
            ESP_LOGE(TAG, "Start of List");

            // Print Buffer
            // for (size_t i = 0; i < buffer_length; ++i) {
            //     uint8_t byte = *(buffer + i); 
            //     printf("%u,", byte);
            // }
            // exportImage(buffer, buffer_length);
            takePicture = false;
            ESP_LOGE(TAG, "Capture Completed");
            allowPicture = true;
            startSleep();
        }


        // Everything below this is not needed

        if (res == ESP_OK)
        {
            res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
        }
        if (res == ESP_OK)
        {
            size_t hlen = snprintf(part_buf, sizeof(part_buf), _STREAM_PART, _jpg_buf_len);
            res = httpd_resp_send_chunk(req, part_buf, hlen);
        }
        if (res == ESP_OK)
        {
            res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
        }
        if (fb->format != PIXFORMAT_JPEG)
        {
            free(_jpg_buf);
        }
        esp_camera_fb_return(fb);
        if (res != ESP_OK)
        {
            break;
        }
        int64_t frame_time = (esp_timer_get_time() - last_frame) / 1000;
        last_frame = esp_timer_get_time();
    }
    last_frame = 0;
    return res;
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

    return ESP_OK;
}

static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        ESP_LOGI(TAG, "Disconnected from Wi-Fi");
        esp_wifi_connect();
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        s_ip_addr = event->ip_info.ip;
        ESP_LOGI(TAG, "Got IP Address: " IPSTR, IP2STR(&s_ip_addr));
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "UCInet Mobile Access", 
            .password = "",
        },
    };

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_start();

    ESP_LOGI(TAG, "Connecting to Wi-Fi...");

    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);
}

void start_camera_server()
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    httpd_handle_t server = NULL;
    if (httpd_start(&server, &config) == ESP_OK)
    {
        httpd_uri_t stream_uri = {
            .uri = "/stream",
            .method = HTTP_GET,
            .handler = jpg_stream_httpd_handler,
            .user_ctx = NULL};
        httpd_register_uri_handler(server, &stream_uri);
    }
    else
    {
        ESP_LOGI(TAG, "Error starting server!");
    }
}



// // task handling 
// static void IRAM_ATTR breakbeam_isr(void *param)
// {
// xSemaphoreGiveFromISR(cameraSem, nullptr);
// }

CameraTask::CameraTask(QueueHandle_t &messageQueue)
    : Task(name, priority, stackSize), mMessageQueue(messageQueue)
{
}

void CameraTask::start()
{
    xTaskCreate(taskFunction, mName, mStackSize, this, mPriority, nullptr);
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
    
    // Flash Pin Declaration (GPIO 4)    
    
    gpio_reset_pin(flashPIN);
    gpio_set_direction(flashPIN, GPIO_MODE_OUTPUT);    
    gpio_set_pull_mode(flashPIN, GPIO_PULLDOWN_ONLY);
    gpio_set_level(flashPIN, 0);

    // Input Pin Declaration (GPIO 14)
    gpio_reset_pin(inputPIN);
    gpio_set_direction(inputPIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(inputPIN, GPIO_PULLDOWN_ONLY); 
    gpio_set_level(inputPIN, 0);
    
    gpio_reset_pin(ledPin1);
    gpio_set_direction(ledPin1, GPIO_MODE_OUTPUT);
    gpio_set_pull_mode(ledPin1, GPIO_PULLDOWN_ONLY);
    gpio_set_level(ledPin1, 0);

    gpio_reset_pin(ledPin2);
    gpio_set_direction(ledPin2, GPIO_MODE_OUTPUT);
    gpio_set_pull_mode(ledPin2, GPIO_PULLDOWN_ONLY);
    gpio_set_level(ledPin2, 0);

    static pwm_task_params_t pwm_params = {
        .pin = ledPin1,    // Replace with your GPIO pin
        .duration_ms = 5000    // Replace with your desired duration in ms
    };
    while(1){
        xTaskCreate(pwm, "PWM Task", 2048, &pwm_params, 1, NULL);
    }

    // Initialize NVS
    ESP_ERROR_CHECK(nvs_flash_init());

    // Initialize the camera
    if (ESP_OK != init_camera())
    {
        return;
    }

    wifi_init_sta();
    start_camera_server();
}
