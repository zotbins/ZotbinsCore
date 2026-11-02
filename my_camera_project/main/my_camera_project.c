// camera test for zotbins, started from chad's my_camera_project.c on the wrover-camera_test-temp branch
//
// what i changed from chad's version:
//   1. added a BOARD_ESP32CAM switch so the same file works on the ai-thinker esp32-cam (what im testing on rn)
//      and on the zotbins wrover pcb (chad's original pin numbers are still there in the #else part)
//   2. added send_jpeg_over_serial() and a small block in camera_task so every 5th frame gets sent over
//      the usb cable as text, capture_photos.py on my laptop catches it and saves it as a .jpg
//   3. everything else (camera settings, init_camera, app_main) is still chad's code, i didnt change it
//
// build notes for whoever runs this next:
//   - i built this with esp-idf 6.1, the esp32-camera driver that was committed (2.1.4) doesnt compile on idf 6
//     so i deleted build/, managed_components/ and dependencies.lock and let it pull 2.1.8 (fix was in 2.1.5)
//   - tested on 1 ai-thinker esp32-cam sitting on an esp32-cam-mb, camera shows up as OV2640 (PID 0x26)
//     and gives 800x600 jpegs around 15-30 KB each

#include <stdio.h>
#include "esp_system.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_camera.h"
#include "driver/gpio.h"

static const char *TAG = "cam";

// i added this switch because the pin numbers have to match whatever board the camera is plugged into
// chad's numbers are for the zotbins wrover pcb but im testing on an ai-thinker esp32-cam which is wired different
// with chad's pins on my board the camera never got detected (error 0x106), the clock was going out on gpio21
// but on the esp32-cam the camera clock input is gpio0, and the power down pin (gpio32) never got pulled low
// comment out the line below to go back to chad's pcb pins
// careful: if this is left on and flashed to the zotbins pcb, the clock goes out on io0 which is the boot
// button on that board, so the camera wont start and dont press boot while its running
#define BOARD_ESP32CAM   // comment out this line to use the ZotBins PCB pins

#ifdef BOARD_ESP32CAM
// ai-thinker esp32-cam pinout, same numbers as camera_sd.c and the old cameraTask.cpp in the zotbins repo
#define CAM_PIN_PWDN    32   // camera power down, the driver pulls this low to turn the camera on
#define CAM_PIN_RESET   -1   // reset isnt wired to a gpio on this board
#define CAM_PIN_XCLK     0   // esp32 sends the camera its clock on this pin
#define CAM_PIN_SIOD    26   // sccb data (basically i2c), used to set the camera registers
#define CAM_PIN_SIOC    27   // sccb clock
#define CAM_PIN_D7      35   // d0-d7 are the 8 pixel data lines
#define CAM_PIN_D6      34   // 34, 35, 36, 39 are input only pins on the esp32 which is fine since data only comes in
#define CAM_PIN_D5      39
#define CAM_PIN_D4      36
#define CAM_PIN_D3      21
#define CAM_PIN_D2      19
#define CAM_PIN_D1      18
#define CAM_PIN_D0       5   // the driver's d0 is the camera's Y2 not Y0, in 8 bit mode the ov2640 puts data on Y9-Y2
#define CAM_PIN_VSYNC   25   // goes high at the start of each frame
#define CAM_PIN_HREF    23   // high while a row of pixels is valid
#define CAM_PIN_PCLK    22   // pixel clock from the camera, one byte per tick
#else
// zotbins wrover pcb pinout, these are chad's original numbers (the wrover-kit default)
// heads up: in the kicad files i have, d0-d7 here connect to the camera's Y0-Y7 instead of Y2-Y9
// and Y8/Y9 arent connected, still asking chad if thats the current board
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
#endif

// Initialize camera
// this whole function is chad's, i didnt change anything in here
static esp_err_t init_camera(void)
{
    // Enable internal pull-ups for SIOD/SIOC
    // note: on idf 6 this is what causes the "GPIO 26/27 is not usable" warnings at boot, the camera driver
    // tries to claim these pins after this already did, it still works fine so i left it
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << CAM_PIN_SIOD) | (1ULL << CAM_PIN_SIOC),
        .mode = GPIO_MODE_INPUT_OUTPUT_OD, 
        .pull_up_en = GPIO_PULLUP_ENABLE,  
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);

    // camera settings, same as chad's
    // 8 MHz clock, jpeg output, svga = 800x600, quality 12 (lower number = better quality but bigger file)
    // 2 frame buffers in psram so the camera can fill one while im using the other
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

// ---------- added by me ----------
// i added this so i can actually see a picture without an sd card or wifi
// it sends one jpeg over the usb cable as hex text between a JPEG_START line and a JPEG_END line
// capture_photos.py on my laptop reads that and turns it back into a .jpg file
// why hex: the serial port is a text connection, raw image bytes get messed up on the way
// (like idf turns any newline byte into 2 bytes), hex only uses 0-9 and a-f so it gets there in one piece
// it does make the data 2x bigger so a 25 KB photo takes around 4-5 s to send at 115200 baud
#define PHOTO_EVERY_N_FRAMES 5   // frames are 2 s apart, so about one photo every 10 s

static void send_jpeg_over_serial(const camera_fb_t *fb, uint32_t photo_num)
{
    static const char hex[] = "0123456789abcdef";   // lookup table, index 0-15 gives the hex digit
    char line[2 * 48 + 1];                           // 48 bytes per line = 96 hex chars, +1 for the '\0'

    // start marker has the photo number and the size so the python script can check nothing got lost
    printf("\nJPEG_START %lu %u\n", (unsigned long)photo_num, (unsigned)fb->len);
    for (size_t i = 0; i < fb->len; i += 48) {
        size_t n = (fb->len - i < 48) ? (fb->len - i) : 48;   // last line can be shorter than 48
        for (size_t j = 0; j < n; j++) {
            line[2 * j]     = hex[fb->buf[i + j] >> 4];     // top 4 bits of the byte
            line[2 * j + 1] = hex[fb->buf[i + j] & 0x0F];   // bottom 4 bits of the byte
        }
        line[2 * n] = '\0';
        printf("%s\n", line);
    }
    printf("JPEG_END\n");
    fflush(stdout);   // push everything out right away instead of leaving it sitting in the buffer
}
// ---------- end of what i added ----------

static void camera_task(void *pvParameters)
{
    uint32_t frame_count = 0;
    uint32_t photo_count = 0;   // added by me, counts how many photos got sent so the files get numbered

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

        // added by me: every 5th frame gets sent to the laptop
        // this also skips the first few frames while auto exposure and white balance settle
        // (the old zotbins camera code waited 30 frames because early ones came out green)
        // has to happen before esp_camera_fb_return() since that hands the buffer back to the driver
        // and after that the photo is gone
        if (frame_count % PHOTO_EVERY_N_FRAMES == 0) {
            photo_count++;
            ESP_LOGI(TAG, "Sending photo %lu over serial...", photo_count);
            send_jpeg_over_serial(fb, photo_count);
        }

        esp_camera_fb_return(fb);
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

// chad's, unchanged
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