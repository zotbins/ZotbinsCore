
#include "gpsTask.hpp"
#include "driver/uart.h"
#include "task.hpp"  
#include "Client.hpp"
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_log.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"

using namespace Zotbins;

static const char *name = "gpsTask";
static const int priority = 2; // Maybe change priority later
static const uint32_t stackSize = 4096;

const gpio_num_t flashPIN = GPIO_NUM_4;

#define GPS_UART_NUM      UART_NUM_0
#define GPS_UART_BAUD     9600
#define GPS_UART_TX_PIN   1
#define GPS_UART_RX_PIN   3
#define GPS_BUFFER_SIZE   1024

void gps_uart_init() {
    uart_config_t uart_config = {
        .baud_rate = GPS_UART_BAUD,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    // Configure UART parameters
    uart_param_config(GPS_UART_NUM, &uart_config);
    uart_set_pin(GPS_UART_NUM, GPS_UART_TX_PIN, GPS_UART_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(GPS_UART_NUM, GPS_BUFFER_SIZE, 0, 0, NULL, 0);
}

void gps_read_data() {
    Client::clientPublishStr("Read");
    return;
    uint8_t data[GPS_BUFFER_SIZE + 1];  // +1 for null-termination
    int len = uart_read_bytes(GPS_UART_NUM, data, GPS_BUFFER_SIZE, 100 / portTICK_PERIOD_MS);

    if (len > 0) {
        data[len] = '\0';  // Null-terminate to form a valid string/
        ESP_LOGI("GPS", "Received: %s", (char*)data);
        uart_write_bytes(UART_NUM_0, (const char*)data, len);  
        char message[2048];
        snprintf(message, sizeof(message), "GPS Data: %s", (char*)data);
        Client::clientPublishStr(message);
    }
}

GpsTask::GpsTask(QueueHandle_t &messageQueue)
    : Task(name, priority, stackSize), mMessageQueue(messageQueue)
{
}

void GpsTask::start()
{
    xTaskCreatePinnedToCore(taskFunction, mName, mStackSize, this, mPriority, nullptr, 1);
}

void GpsTask::taskFunction(void *task)
{
    GpsTask *gpsTask = static_cast<GpsTask *>(task);
    gpsTask->setup();
    gpsTask->loop();
}

void GpsTask::setup()
{
}



void GpsTask::loop()
{

    gpio_reset_pin(flashPIN);
    gpio_set_direction(flashPIN, GPIO_MODE_OUTPUT);
    gpio_set_pull_mode(flashPIN, GPIO_PULLDOWN_ONLY);
    gpio_set_level(flashPIN, 0);

    gps_uart_init();

    Client::clientPublishStr("Started");
    gps_read_data();
    Client::clientPublishStr("Finished");


    while(1){
        gps_read_data();
        ESP_LOGE(name, "GPS Working");
        Client::clientPublishStr("Amongus");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
