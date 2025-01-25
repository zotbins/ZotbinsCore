#define BOARD_WROVER_KIT
#define BOARD_ESP32CAM_AITHINKER

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include <string.h>
#include "driver/i2c.h"
#include "unity.h"
#include <mbedtls/base64.h>

#include "esp_http_server.h"
#include "esp_camera.h"
#include "esp_timer.h"
#include "esp_mac.h"
#include "esp_spiffs.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_event.h"
#include <esp_system.h>
#include <esp_log.h>
#include <cJSON.h>

#include <nvs_flash.h>
#include <sys/param.h>

#define WIFI_SSID      "UCInet Mobile Access"   //or  or VDCNorte-MyCampusNet
#define WIFI_PASS      ""

#ifndef portTICK_RATE_MS
#define portTICK_RATE_MS portTICK_PERIOD_MS
#endif

// ESP32Cam (AiThinker) PIN Map
#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1 //software reset will be performed
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27

#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define LED_GPIO_NUM 4
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22

#define I2C_MASTER_SCL_IO           4      /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO           5      /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM              0      /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
#define I2C_MASTER_FREQ_HZ          100000 /*!< I2C master clock frequency */

static const char *TAG = "wifi_station";
static EventGroupHandle_t s_wifi_event_group;
esp_netif_t *sta_netif = NULL;

typedef void (*decode_func_t)(uint8_t *jpegbuffer, uint32_t size, uint8_t *outbuffer);

static esp_err_t init_camera(uint32_t xclk_freq_hz, pixformat_t pixel_format, framesize_t frame_size, uint8_t fb_count, int sccb_sda_gpio_num, int sccb_port)
{
    framesize_t size_bak = frame_size;
    if (PIXFORMAT_JPEG == pixel_format && FRAMESIZE_SVGA > frame_size) {
        frame_size = FRAMESIZE_HD;
    }
    camera_config_t camera_config = {
        .pin_pwdn = PWDN_GPIO_NUM,
        .pin_reset = RESET_GPIO_NUM,
        .pin_xclk = XCLK_GPIO_NUM,
        .pin_sccb_sda = sccb_sda_gpio_num, // If pin_sccb_sda is -1, sccb will use the already initialized i2c port specified by `sccb_i2c_port`.
        .pin_sccb_scl = SIOC_GPIO_NUM,
        .sccb_i2c_port = sccb_port,

        .pin_d7 = Y9_GPIO_NUM,
        .pin_d6 = Y8_GPIO_NUM,
        .pin_d5 = Y7_GPIO_NUM,
        .pin_d4 = Y6_GPIO_NUM,
        .pin_d3 = Y5_GPIO_NUM,
        .pin_d2 = Y4_GPIO_NUM,
        .pin_d1 = Y3_GPIO_NUM,
        .pin_d0 = Y2_GPIO_NUM,
        .pin_vsync = VSYNC_GPIO_NUM,
        .pin_href = HREF_GPIO_NUM,
        .pin_pclk = PCLK_GPIO_NUM,

        //EXPERIMENTAL: Set to 16MHz on ESP32-S2 or ESP32-S3 to enable EDMA mode
        .xclk_freq_hz = xclk_freq_hz,
        .ledc_timer = LEDC_TIMER_0,
        .ledc_channel = LEDC_CHANNEL_0,

        .pixel_format = pixel_format, //YUV422,GRAYSCALE,RGB565,JPEG
        .frame_size = frame_size,    //QQVGA-UXGAQQVGA-UXGA, For ESP32, do not use sizes above QVGA when not JPEG. The performance of the ESP32-S series has improved a lot, but JPEG mode always gives better frame rates.

        .jpeg_quality = 0, //0-63, for OV series camera sensors, lower number means higher quality
        .fb_count = fb_count,       //When jpeg mode is used, if fb_count more than one, the driver will work in continuous mode.
        .grab_mode = CAMERA_GRAB_WHEN_EMPTY
    };

    //initialize the camera
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Camera Init Failed with error 0x%x", err);
        return err;
    }

    sensor_t *s = esp_camera_sensor_get();

    // TODO: these filters are not working properly, keeps tinting the camera yellow
    s->set_whitebal(s, 1); // 1 enables auto white balance
    s->set_colorbar(s, 0); // Disable color bar
    s->set_exposure_ctrl(s, 1); // Set to auto exposure

    if (ESP_OK == err && PIXFORMAT_JPEG == pixel_format && FRAMESIZE_SVGA > size_bak) {
        sensor_t *s = esp_camera_sensor_get();
        if (s->id.PID != OV2640_PID) {
            ESP_LOGE(TAG, "Unsupported camera sensor detected");
            return !err;
        }
        s->set_framesize(s, size_bak);
    }

    return err;
}

void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "Got IP Address: " IPSTR, IP2STR(&event->ip_info.ip));
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        ESP_LOGI(TAG, "Retrying connection to the AP...");
    }
}

void wifi_init_sta() {
    s_wifi_event_group = xEventGroupCreate();

    // Initialize NVS (Non-Volatile Storage) for Wi-Fi settings
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    // Initialize Wi-Fi with default settings
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Create a default Wi-Fi station (STA) network interface
    sta_netif = esp_netif_create_default_wifi_sta();
    if (sta_netif == NULL) {
        ESP_LOGE(TAG, "Failed to create default Wi-Fi STA");
        return;
    }

    // Initialize the Wi-Fi driver
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // Register event handler
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));

    // Configure Wi-Fi in station mode
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };

    // Start Wi-Fi
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Wi-Fi Station initialized");
}

char *encode_image_to_base64(const camera_fb_t *fb) {
    size_t encoded_length = 0;
    size_t output_buffer_size = (fb->len * 4 / 3) + 4; // Base64 encoding increases size by ~33%

    // Allocate memory for Base64 encoded output
    char *encoded_output = (char *)malloc(output_buffer_size);
    if (!encoded_output) {
        ESP_LOGE("BASE64", "Failed to allocate memory for Base64 encoding");
        return NULL;
    }

    // Encode the image data to Base64
    int ret = mbedtls_base64_encode((unsigned char *)encoded_output, output_buffer_size, &encoded_length, fb->buf, fb->len);
    if (ret != 0) {
        ESP_LOGE("BASE64", "Failed to encode image to Base64");
        free(encoded_output);
        return NULL;
    }

    return encoded_output;
}

void save_image_to_json(const char *encoded_image) {
    // Create a JSON object
    cJSON *json = cJSON_CreateObject();
    if (!json) {
        ESP_LOGE("cJSON", "Failed to create JSON object");
        return;
    }

    // Add the Base64-encoded image to the JSON object
    cJSON_AddStringToObject(json, "image", encoded_image);

    // Convert the JSON object to a string
    char *json_string = cJSON_Print(json);
    if (!json_string) {
        ESP_LOGE("cJSON", "Failed to print JSON object");
        cJSON_Delete(json);
        return;
    }

    // Save the JSON string to a file (assuming SPIFFS or other filesystem is mounted)
    FILE *file = fopen("/spiffs/image.json", "w");
    if (!file) {
        ESP_LOGE("File", "Failed to open file for writing");
        free(json_string);
        cJSON_Delete(json);
        return;
    }

    //ESP_LOGI(TAG,"%s\n", json_string);
    fprintf(file,"%s", json_string);
    fclose(file);

    // Cleanup
    free(json_string);
    cJSON_Delete(json);
}

// Code segment taken from ESP32 Camera driver
#define PART_BOUNDARY "123456789000000000000987654321"
static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";
esp_err_t jpg_stream_httpd_handler(httpd_req_t *req){
    camera_fb_t * fb = NULL;
    esp_err_t res = ESP_OK;
    size_t _jpg_buf_len;
    uint8_t * _jpg_buf;
    char * part_buf[64];
    static int64_t last_frame = 0;
    if(!last_frame) {
        last_frame = esp_timer_get_time();
    }

    res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
    if(res != ESP_OK){
        return res;
    }

    while(true){
        fb = esp_camera_fb_get();
        if (!fb) {
            ESP_LOGE(TAG, "Camera capture failed");
            res = ESP_FAIL;
            break;
        }
        if(fb->format != PIXFORMAT_JPEG){
            bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
            if(!jpeg_converted){
                ESP_LOGE(TAG, "JPEG compression failed");
                esp_camera_fb_return(fb);
                res = ESP_FAIL;
            }
        } else {
            _jpg_buf_len = fb->len;
            _jpg_buf = fb->buf;
        }

        if(res == ESP_OK){
            res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
        }
        if(res == ESP_OK){
            size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART, _jpg_buf_len);

            res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
        }
        if(res == ESP_OK){
            res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
        }
        if(fb->format != PIXFORMAT_JPEG){
            free(_jpg_buf);
        }
        esp_camera_fb_return(fb);
        if(res != ESP_OK){
            break;
        }
        int64_t fr_end = esp_timer_get_time();
        int64_t frame_time = fr_end - last_frame;
        last_frame = fr_end;
        frame_time /= 1000;
        // ESP_LOGI(TAG, "MJPG: %uKB %ums (%.1ffps)",
        //     (uint32_t)(_jpg_buf_len/1024),
        //     (uint32_t)frame_time, 1000.0 / (uint32_t)frame_time);
    }

    last_frame = 0;
    return res;
}




// void init_spiffs() {
//     esp_vfs_spiffs_conf_t conf = {
//         .base_path = "/spiffs",
//         .partition_label = NULL,
//         .max_files = 5,
//         .format_if_mount_failed = true
//     };
    
//     esp_err_t ret = esp_vfs_spiffs_register(&conf);
//     if (ret != ESP_OK) {
//         if (ret == ESP_FAIL) {
//             ESP_LOGE("SPIFFS", "Failed to mount or format filesystem");
//         } else if (ret == ESP_ERR_NOT_FOUND) {
//             ESP_LOGE("SPIFFS", "Failed to find SPIFFS partition");
//         } else {
//             ESP_LOGE("SPIFFS", "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
//         }
//         return;
//     }

//     size_t total = 0, used = 0;
//     ret = esp_spiffs_info(NULL, &total, &used);
//     if (ret != ESP_OK) {
//         ESP_LOGE("SPIFFS", "Failed to get SPIFFS partition information");
//     } else {
//         ESP_LOGI("SPIFFS", "Partition size: total: %d, used: %d", total, used);
//     }
// }

//EVERYTHING BELOW THIS are http fxns (except app_main is simply to view image for quality check)

typedef struct {
        httpd_req_t *req;
        size_t len;
} jpg_chunking_t;

static size_t jpg_encode_stream(void * arg, size_t index, const void* data, size_t len){
    jpg_chunking_t *j = (jpg_chunking_t *)arg;
    if(!index){
        j->len = 0;
    }
    if(httpd_resp_send_chunk(j->req, (const char *)data, len) != ESP_OK){
        return 0;
    }
    j->len += len;
    return len;
}

esp_err_t jpg_httpd_handler(httpd_req_t *req){
    camera_fb_t * fb = NULL;
    esp_err_t res = ESP_OK;
    size_t fb_len = 0;
    int64_t fr_start = esp_timer_get_time();

    fb = esp_camera_fb_get();
    if (!fb) {
        ESP_LOGE(TAG, "Camera capture failed");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    res = httpd_resp_set_type(req, "image/jpeg");
    if(res == ESP_OK){
        res = httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=capture.jpg");
    }

    if(res == ESP_OK){
        if(fb->format == PIXFORMAT_JPEG){
            fb_len = fb->len;
            res = httpd_resp_send(req, (const char *)fb->buf, fb->len);
        } else {
            jpg_chunking_t jchunk = {req, 0};
            res = frame2jpg_cb(fb, 80, jpg_encode_stream, &jchunk)?ESP_OK:ESP_FAIL;
            httpd_resp_send_chunk(req, NULL, 0);
            fb_len = jchunk.len;
        }
    }
    esp_camera_fb_return(fb);
    int64_t fr_end = esp_timer_get_time();
    // ESP_LOGI(TAG, "JPG: %uKB %ums", (uint32_t)(fb_len/1024), (uint32_t)((fr_end - fr_start)/1000));
    return res;
}


// // HTTP GET handler for serving the captured image
// esp_err_t capture_handler(httpd_req_t *req) {
//     // Capture image from camera
//     camera_fb_t *fb = esp_camera_fb_get();
//     if (!fb) {
//         ESP_LOGE(TAG, "Failed to capture image");
//         httpd_resp_send_500(req);
//         return ESP_FAIL;
//     }

//     // Send response headers
//     httpd_resp_set_type(req, "image/jpeg");
//     httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=capture.jpg");

//     // Send the image as the HTTP response
//     esp_err_t res = httpd_resp_send(req, (const char *)fb->buf, fb->len);

//     // Return the frame buffer back to the driver
//     esp_camera_fb_return(fb);

//     if (res != ESP_OK) {
//         ESP_LOGE(TAG, "Failed to send the image");
//     }

//     return res;
// }



// Start the HTTP server and set up URI handler
void start_camera_server() {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;

    // Start the server
    if (httpd_start(&server, &config) == ESP_OK) {
        // URI handler to capture and serve the image
        httpd_uri_t capture_uri = {
            .uri       = "/capture",
            .method    = HTTP_GET,
            .handler   = jpg_stream_httpd_handler, // jpg_httpd_handler,
            .user_ctx  = NULL
        };

        httpd_register_uri_handler(server, &capture_uri);
        ESP_LOGI(TAG, "Server started, you can access the image at /capture");
    } else {
        ESP_LOGE(TAG, "Failed to start the server");
    }
}

//ONLY NEEDED FOR CONNECTING TO OTHER WIFI SPOTS
void print_mac_address() {
    uint8_t mac[6];
    // Get the MAC address for the Wi-Fi station interface
    //esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, mac);
    esp_err_t ret = esp_efuse_mac_get_default(mac);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "MAC Address (STA): %02x:%02x:%02x:%02x:%02x:%02x", 
                 mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    } else {
        ESP_LOGE(TAG, "Failed to get MAC address");
    }
}

void app_main() {
    // turn on camera flash
    esp_rom_gpio_pad_select_gpio(LED_GPIO_NUM);
    gpio_set_direction(LED_GPIO_NUM, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_GPIO_NUM, 1);

    
    // Initialize the camera
    TEST_ESP_OK(init_camera(20000000, PIXFORMAT_RGB565, FRAMESIZE_QVGA, 2, SIOD_GPIO_NUM, -1));

    // Initialize NVS (non-volatile storage) flash memory
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    // print_mac_address();

    // Initialize the Wi-Fi station
    wifi_init_sta();

    // Start the HTTP server to serve images
    start_camera_server();

    // turn off flash
    vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay for 1 second
    gpio_set_level(LED_GPIO_NUM, 0);
}

/*void app_main(void)
{

    #if ESP_CAMERA_SUPPORTED

    TEST_ESP_OK(init_camera(20000000, PIXFORMAT_RGB565, FRAMESIZE_QVGA, 2, SIOD_GPIO_NUM, -1));
    vTaskDelay(500 / portTICK_RATE_MS);
    //while (1)
    //{
    init_spiffs();

    ESP_LOGI(TAG, "Taking picture...");
    camera_fb_t *pic = esp_camera_fb_get();
    if (!pic) {
        ESP_LOGE("CAMERA", "Camera capture failed");
        return;
    }

    // Convert image to Base64
    char *encoded_image = encode_image_to_base64(pic);
    if (!encoded_image) {
        ESP_LOGE("BASE64", "Failed to encode image");
        return;
    }

    save_image_to_json(encoded_image);

    ESP_LOGI(TAG, "Picture taken! Its size was: %zu bytes", pic->len);
    esp_camera_fb_return(pic);

    vTaskDelay(5000 / portTICK_RATE_MS);
    //}
#else
    ESP_LOGE(TAG, "Camera support is not available for this chip");
    return;
#endif
}*/

//HI