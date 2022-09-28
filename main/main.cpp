#include "fullnessTask.hpp"
#include "message.hpp"
#include "usageTask.hpp"
#include "weightTask.hpp"
#include "wifiTask.hpp"
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <hx711.h>
#define ZBIN_CORE_VERSION "0.0.1"
#define ZBIN_DISABLE_WIFI

constexpr size_t messageQueueSize = 20;
static const char *TAG = "Main";

void setup();
void loop();

extern "C" void app_main(void)
{
   setup();
   loop();
}

void testFunc(void* param)
{

    hx711_t dev = {
        .dout = GPIO_NUM_19,
        .pd_sck = GPIO_NUM_18,
        .gain = HX711_GAIN_A_64
    };

    ESP_ERROR_CHECK(hx711_init(&dev));

    while (1)
    {
        esp_err_t r = hx711_wait(&dev, 500);
        if (r != ESP_OK)
        {
            log_e("Device not found: %d (%s)\n", r, esp_err_to_name(r));
            continue;
        }

        int32_t data;
        r = hx711_read_average(&dev, 10, &data);
        if (r != ESP_OK)
        {
            log_e("Could not read data: %d (%s)\n", r, esp_err_to_name(r));
            continue;
        }

        log_i("Raw data: %d\n", data);

        vTaskDelay(pdMS_TO_TICKS(500));
    }
    /*
    HX711 sensor{};
    #define IS_FREE_RTOS defined(ARDUINO_ARCH_ESP32)
    #if IS_FREE_RTOS
    log_i("IS FREE RTOS???");
    #endif
    sensor.begin(16, 4);
    sensor.set_scale(20);
    if (sensor.is_ready())
        {
            log_i("READY!");
        }
        else
        {
            log_e("NOT READY");
        }

    sensor.tare(10);
    while (1)
    {
        if (sensor.is_ready())
        {
            log_i("READY!");
        }
        else
        {
            log_e("NOT READY");
        }
        log_i("%d", sensor.get_offset());
        log_i("%ld", (sensor.read_average(10) - sensor.get_offset())/ sensor.get_scale());
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    */
}

void setup()
{
    xTaskCreate(testFunc, "Test", 4096, nullptr, 1, nullptr);
    log_i("ZotBins Core Version: %s", ZBIN_CORE_VERSION);

    // Create message queue for inter-task communication
    QueueHandle_t messageQueue = xQueueCreate(messageQueueSize, sizeof(Zotbins::Message));
    assert(messageQueue != nullptr);

    // Create tasks
    Zotbins::FullnessTask fullnessTask(messageQueue);
    Zotbins::UsageTask usageTask(messageQueue);
    Zotbins::WeightTask weightTask(messageQueue);
#ifndef ZBIN_DISABLE_WIFI
    Zotbins::WiFiTask wifiTask(messageQueue);
#endif

    // Start all tasks
    //fullnessTask.start();
    //usageTask.start();
    //weightTask.start();
#ifndef ZBIN_DISABLE_WIFI
    wifiTask.start();
#endif
}

void loop()
{
    // Do nothing. Things will be done in tasks
}