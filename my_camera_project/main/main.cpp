// main.cpp, camera test for the new zotbins board (aurnob's board)
//
// what this program does:
//   - turns on the OV2640, throws away the first few frames so the picture can settle
//   - then grabs a jpeg every 2 s, checks it really is a jpeg and logs its size
//   - every 5th frame it sends the jpeg over the usb cable as hex text
//   - capture_photos.py on my laptop catches that text and saves it as a .jpg
//
// how the camera pieces fit together:
//   1. the esp32 sends the camera a clock (XCLK), the camera cant do anything without it
//   2. the esp32 talks to the camera's settings registers over SCCB, which is basically i2c
//      (SIOD = data, SIOC = clock), the driver uses it to read the camera id and set it up for jpeg
//   3. the camera sends the picture back on 8 data lines (D0-D7), one byte per PCLK tick
//      VSYNC marks the start of a frame and HREF is high while a row of good data is coming out
//   4. the driver uses DMA to copy those bytes into a frame buffer in psram, then i read it from there
//
//   - stuff i kept from my version: pin checks when it compiles, sending every 5th photo to the laptop,
//     and the send loop that pauses so the watchdog doesnt go off
//
// build notes for whoever runs this next:
//   - built with esp-idf 6.1 and esp32-camera 2.1.8 (pinned in dependencies.lock)
//   - the pins come from aurnob's schematic (U1 esp32 page + J5), my teammate got the same 16 numbers
//     on his own, but nothing here has run on the real board yet

#include <stdio.h>               // printf, i use it to send the photo as text

// freertos is the os esp-idf runs on, the camera driver uses its tasks and queues too
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"       // xTaskCreate and vTaskDelay

#include "esp_camera.h"          // the esp32-camera driver (comes from main/idf_component.yml)
#include "esp_log.h"             // ESP_LOGI / ESP_LOGW / ESP_LOGE, prints with a tag and timestamp
#include "esp_psram.h"           // lets me check if the psram chip was found at boot
#include "esp_err.h"             // esp_err_to_name, turns error codes into readable names

static const char *TAG = "cam";  // shows up in front of every log line from this file, like "I (1454) cam: ..."

// ---------- pins ----------
// which esp32 gpio each camera signal is wired to on the new board, -1 means not connected to the esp32
// outputs from the esp32: XCLK and SIOC, inputs to the esp32: D0-D7, VSYNC, HREF, PCLK
// SIOD goes both ways (open drain like i2c sda)
// same static const int style as my teammate, in c++ these count as compile time numbers so the
// pin checks below still work on them
// big change from the old kicad: D0-D7 here are the camera's Y2-Y9 (DATA2-DATA9), DATA0/DATA1 arent
// connected, thats the right way for 8 bit mode
static const int CAM_PIN_PWDN = -1;   // pwdn is tied to gnd on the board so the camera is always on
static const int CAM_PIN_RESET = -1;  // reset is an rc circuit on the board (R13 + C14), not a gpio
static const int CAM_PIN_XCLK = 33;   // XCLK1, TP6 is on this net so i can scope the 8 MHz
static const int CAM_PIN_SIOD = 26;   // sccb data, TP4
static const int CAM_PIN_SIOC = 27;   // sccb clock, TP5
static const int CAM_PIN_D7 = 23;     // DATA9 (Y9), most significant bit of each byte
static const int CAM_PIN_D6 = 22;     // DATA8 (Y8)
static const int CAM_PIN_D5 = 21;     // DATA7 (Y7)
static const int CAM_PIN_D4 = 5;      // DATA6 (Y6)
static const int CAM_PIN_D3 = 19;     // DATA5 (Y5)
static const int CAM_PIN_D2 = 34;     // DATA4 (Y4), gpio34-39 are input only but thats fine for data
static const int CAM_PIN_D1 = 35;     // DATA3 (Y3)
static const int CAM_PIN_D0 = 18;     // DATA2 (Y2), least significant bit
static const int CAM_PIN_VSYNC = 25;  // pulses once per frame
static const int CAM_PIN_HREF = 39;   // SENSOR_VN, high while a row of pixels is being sent
static const int CAM_PIN_PCLK = 36;   // SENSOR_VP, one tick per byte, the esp32 reads D0-D7 on this
// heads up: esp32 errata says gpio36/39 get pulled low for ~80 ns when sar adc1, sar adc2 or amp gets powered on
// pclk and href are on those two pins on this board so dont turn the adc on while the camera is capturing

// ---------- pin checks ----------
// these run when the code compiles, not on the board, so the build stops if a pin number breaks an esp32 rule
// they dont prove the board is wired right (thats the schematic check), they just catch typos and bad pins
// constexpr means the compiler works these out while building, so static_assert can use them

// is this a real esp32 gpio that i'm allowed to use
static constexpr bool gpio_ok(int p)
{
    bool exists = p >= 0 && p <= 39 && p != 20 && p != 24 && !(p >= 28 && p <= 31);  // esp32 has no gpio 20, 24, 28-31
    bool reserved = (p >= 6 && p <= 11) || p == 16 || p == 17;  // 6-11 are the flash pins, 16/17 are psram on a wrover
    return exists && !reserved;
}

// can this pin drive an output (34-39 are input only)
static constexpr bool gpio_can_output(int p)
{
    return gpio_ok(p) && p < 34;
}

// checks that no two camera signals got put on the same gpio
static constexpr bool pins_all_different()
{
    const int pins[] = {CAM_PIN_XCLK, CAM_PIN_SIOD, CAM_PIN_SIOC,
                        CAM_PIN_D0, CAM_PIN_D1, CAM_PIN_D2, CAM_PIN_D3,
                        CAM_PIN_D4, CAM_PIN_D5, CAM_PIN_D6, CAM_PIN_D7,
                        CAM_PIN_VSYNC, CAM_PIN_HREF, CAM_PIN_PCLK};
    const int count = sizeof(pins) / sizeof(pins[0]);
    for (int i = 0; i < count; i++) {
        for (int j = i + 1; j < count; j++) {
            if (pins[i] == pins[j]) {
                return false;  // found two signals on the same pin
            }
        }
    }
    return true;
}

static_assert(gpio_can_output(CAM_PIN_XCLK), "xclk has to be a pin that can output");
static_assert(gpio_can_output(CAM_PIN_SIOD) && gpio_can_output(CAM_PIN_SIOC), "sccb pins have to be able to output");
static_assert(gpio_ok(CAM_PIN_D0) && gpio_ok(CAM_PIN_D1) && gpio_ok(CAM_PIN_D2) && gpio_ok(CAM_PIN_D3) &&
              gpio_ok(CAM_PIN_D4) && gpio_ok(CAM_PIN_D5) && gpio_ok(CAM_PIN_D6) && gpio_ok(CAM_PIN_D7),
              "a data pin is not a usable gpio");
static_assert(gpio_ok(CAM_PIN_VSYNC) && gpio_ok(CAM_PIN_HREF) && gpio_ok(CAM_PIN_PCLK),
              "vsync, href or pclk is not a usable gpio");
static_assert(pins_all_different(), "two camera signals are on the same gpio");

// ---------- settings ----------
static const int XCLK_FREQ_HZ = 8000000;          // 8 MHz, what we tested with, the ov2640 datasheet allows 6-24 MHz
static const int WARMUP_FRAMES = 5;               // frames i throw away at the start so auto exposure / white balance can settle
static const int FRAME_DELAY_MS = 2000;           // wait between frames
static const uint32_t PHOTO_EVERY_N_FRAMES = 5;   // send every 5th frame to the laptop, so about one photo every 10 s
static const size_t HEX_BYTES_PER_LINE = 48;      // 48 bytes = 96 hex chars per line
static const uint32_t LINES_BETWEEN_PAUSES = 32;  // after this many lines the send loop pauses for 1 tick

// ---------- camera setup ----------
// start_camera() fills in the camera settings and starts the driver
// returns ESP_OK if the camera answered, otherwise the error from the driver
// same steps as my teammate's start_camera(), i just set the frame buffer count for my loop
static esp_err_t start_camera(void)
{
    // camera_config_t holds everything the driver needs, "= {}" zeroes every field first
    // so anything i dont set below gets a safe default
    // i dont set up the sccb pull-ups myself anymore, the driver turns on the esp32's internal
    // pull-ups on SIOD/SIOC by itself (chad's old gpio_config did it twice and caused the
    // "GPIO 26/27 is not usable" warnings at boot)
    camera_config_t config = {};

    // control pins
    config.pin_pwdn = CAM_PIN_PWDN;     // power down, active high (high = camera off)
    config.pin_reset = CAM_PIN_RESET;   // reset, active low (low = camera held in reset)
    config.pin_xclk = CAM_PIN_XCLK;     // clock the esp32 makes for the camera

    // sccb (i2c like) bus for reading and writing the camera's registers
    config.pin_sccb_sda = CAM_PIN_SIOD;
    config.pin_sccb_scl = CAM_PIN_SIOC;

    // 8 bit data bus, D7 is the most significant bit of each byte
    config.pin_d7 = CAM_PIN_D7;
    config.pin_d6 = CAM_PIN_D6;
    config.pin_d5 = CAM_PIN_D5;
    config.pin_d4 = CAM_PIN_D4;
    config.pin_d3 = CAM_PIN_D3;
    config.pin_d2 = CAM_PIN_D2;
    config.pin_d1 = CAM_PIN_D1;
    config.pin_d0 = CAM_PIN_D0;

    // timing signals that tell the esp32 when the data bus has good pixels on it
    config.pin_vsync = CAM_PIN_VSYNC;
    config.pin_href = CAM_PIN_HREF;
    config.pin_pclk = CAM_PIN_PCLK;

    // xclk is made with the esp32's ledc (pwm) hardware so it needs a timer and a channel
    config.xclk_freq_hz = XCLK_FREQ_HZ;
    config.ledc_timer = LEDC_TIMER_0;
    config.ledc_channel = LEDC_CHANNEL_0;

    // picture settings
    config.pixel_format = PIXFORMAT_JPEG;          // the camera compresses the picture itself
    config.jpeg_quality = 12;                      // 0-63, lower is better quality but a bigger file
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;     // only grab a new frame once ive given the old one back

    // a frame buffer is too big for the esp32's internal ram at 800x600, so it goes in psram
    // the wrover on this board always has psram, so if its missing thats a hardware problem
    // (same fallback as my teammate's so the program still runs, but i log it as an error so its hard to miss)
    if (esp_psram_is_initialized()) {
        config.frame_size = FRAMESIZE_SVGA;        // 800x600
        config.fb_location = CAMERA_FB_IN_PSRAM;
        config.fb_count = 2;                       // 2 buffers since i grab frames in a loop, tested on the esp32-cam
    } else {
        ESP_LOGE(TAG, "No PSRAM found, this board should have it, check the module (using 320x240 for now)");
        config.frame_size = FRAMESIZE_QVGA;        // 320x240 fits in internal ram
        config.fb_location = CAMERA_FB_IN_DRAM;
        config.fb_count = 1;                       // only room for 1 buffer in internal ram
    }

    // starts xclk, finds the camera over sccb (the ov2640's product id is 0x26),
    // writes its setup registers and sets up dma for the data bus
    esp_err_t status = esp_camera_init(&config);
    if (status != ESP_OK) {
        // ESP_ERR_NOT_SUPPORTED (0x106) here means no supported camera answered
        ESP_LOGE(TAG, "Camera init failed: %s (check SCCB wiring, XCLK, and camera power)", esp_err_to_name(status));
        return status;
    }

    // the sensor handle gives access to the camera's id and settings (brightness, flip, etc)
    // i check for nullptr just in case, my teammate's version reads it straight away
    sensor_t *sensor = esp_camera_sensor_get();
    if (sensor == nullptr) {
        ESP_LOGE(TAG, "Camera started but I couldn't get the sensor handle");
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "Camera on, sensor PID 0x%02x", sensor->id.PID);
    return ESP_OK;
}

// ---------- checking a frame ----------
// is_jpeg() checks the first 2 bytes, every jpeg starts with FF D8
// if it doesnt, the data lines are probably giving bad bytes (idea from my teammate's code)
static bool is_jpeg(const camera_fb_t *fb)
{
    return fb->len >= 2 && fb->buf[0] == 0xFF && fb->buf[1] == 0xD8;
}

// ---------- sending photos over usb ----------
// send_jpeg_over_serial() prints one photo as hex text between a JPEG_START line and a JPEG_END line
// capture_photos.py on my laptop reads that and turns it back into a .jpg file
// why hex: the serial port is a text connection, raw bytes get messed up on the way
// (like idf turns any newline byte into 2 bytes), hex only uses 0-9 and a-f so it gets there in one piece
// it does make the data 2x bigger so a 25 KB photo takes around 4-5 s to send at 115200 baud
// fb is the frame from the camera, photo_num is the number that goes in the file name on my laptop
static void send_jpeg_over_serial(const camera_fb_t *fb, uint32_t photo_num)
{
    static const char hex[] = "0123456789abcdef";   // lookup table, index 0-15 gives the hex digit
    char line[2 * HEX_BYTES_PER_LINE + 1];          // 96 hex chars + 1 for the '\0' at the end
    uint32_t lines_sent = 0;

    // start marker has the photo number and the size so the python script can check nothing got lost
    printf("\nJPEG_START %lu %u\n", (unsigned long)photo_num, (unsigned)fb->len);
    for (size_t i = 0; i < fb->len; i += HEX_BYTES_PER_LINE) {
        // last line can be shorter than 48 bytes
        size_t n = (fb->len - i < HEX_BYTES_PER_LINE) ? (fb->len - i) : HEX_BYTES_PER_LINE;
        for (size_t j = 0; j < n; j++) {
            line[2 * j]     = hex[fb->buf[i + j] >> 4];     // top 4 bits of the byte
            line[2 * j + 1] = hex[fb->buf[i + j] & 0x0F];   // bottom 4 bits of the byte
        }
        line[2 * n] = '\0';
        printf("%s\n", line);

        // watchdog fix: printing to the console without the uart driver is a busy wait, the cpu just spins
        // until there's room in the uart fifo and never lets other tasks run
        // a big photo takes 5+ s at 115200 baud and the task watchdog is 5 s, so the idle task would get
        // starved and print "task watchdog got triggered", pausing 1 tick every 32 lines lets it run
        // (1 tick is 10 ms here so it only adds like 0.2 s per photo)
        if (++lines_sent % LINES_BETWEEN_PAUSES == 0) {
            fflush(stdout);
            vTaskDelay(1);
        }
    }
    printf("JPEG_END\n");
    fflush(stdout);   // push everything out right away instead of leaving it sitting in the buffer
}

// ---------- main loop ----------
// camera_task() runs forever: grab a frame, check it, log it, send every 5th one, give it back, wait 2 s
static void camera_task(void *pvParameters)
{
    uint32_t frame_count = 0;   // how many real frames ive grabbed (not counting warm up)
    uint32_t photo_count = 0;   // how many photos got sent, used to number the files

    // the first frames after power up usually come out too dark or green, so i throw a few away
    // (same as my teammate's warm up loop, the old zotbins code waited 30 frames for the same reason)
    for (int i = 0; i < WARMUP_FRAMES; i++) {
        camera_fb_t *fb = esp_camera_fb_get();
        if (fb != nullptr) {
            esp_camera_fb_return(fb);   // hand it straight back so the driver can fill it again
        }
    }
    ESP_LOGI(TAG, "Warm up done, starting frame capture...");

    while (true) {
        camera_fb_t *fb = esp_camera_fb_get();   // borrow a filled frame buffer from the driver
        if (fb == nullptr) {
            // no frame came back (timeout or bad frame), wait and try again instead of crashing
            ESP_LOGE(TAG, "Failed to take a picture");
            vTaskDelay(pdMS_TO_TICKS(FRAME_DELAY_MS));
            continue;
        }

        frame_count++;

        // bad start bytes means the data lines are probably wired or soldered wrong
        // i still send it below if its a photo frame so i can look at what came through
        if (!is_jpeg(fb)) {
            ESP_LOGE(TAG, "Frame #%lu is not a valid JPEG (starts with %02x %02x), check data lines D0-D7",
                     (unsigned long)frame_count, fb->len > 0 ? fb->buf[0] : 0, fb->len > 1 ? fb->buf[1] : 0);
        }

        ESP_LOGI(TAG, "Frame #%lu: %dx%d, %u bytes",
                 (unsigned long)frame_count, (int)fb->width, (int)fb->height, (unsigned)fb->len);

        // every 5th frame gets sent to the laptop
        // this has to happen before esp_camera_fb_return() because after that the buffer belongs
        // to the driver again and the photo is gone
        if (frame_count % PHOTO_EVERY_N_FRAMES == 0) {
            photo_count++;
            ESP_LOGI(TAG, "Sending photo %lu over serial...", (unsigned long)photo_count);
            send_jpeg_over_serial(fb, photo_count);
        }

        esp_camera_fb_return(fb);                      // always give the buffer back when im done with it
        vTaskDelay(pdMS_TO_TICKS(FRAME_DELAY_MS));     // wait before the next frame
    }
}

// app_main() is where esp-idf starts my code after boot, like main() on a pc
// extern "C" is needed because this is a c++ file and esp-idf looks for the plain c name
// it starts the camera, then hands off to camera_task for the loop
extern "C" void app_main(void)
{
    if (start_camera() != ESP_OK) {
        // if the camera didnt start theres no point running the loop, so it just stops here
        return;
    }

    // camera_task gets its own freertos task: 4096 byte stack (printf needs a decent amount), priority 5
    xTaskCreate(camera_task, "camera_task", 4096, nullptr, 5, nullptr);
}
