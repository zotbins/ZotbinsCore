#include "DistanceBuffer.hpp"
#include <vector>
#include <esp_timer.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_intr_alloc.h>
#include <esp_log.h>

#include <stdio.h>
#include <stdbool.h>
#include <ultrasonic.h>
#include <esp_err.h>

#include "ultrasonic.h"

using namespace Fullness;

static const char *name = "DistanceBuffer";

// distance buffer object is associated with a sensor, in this case an ultrasonic sensor under the IDistance class. This is why it requires arguments such as the trigger/echo pins

// DistanceBuffer::DistanceBuffer(gpio_num_t trigger, gpio_num_t echo, std::vector<int32_t> distance)
//     : IDistance(), mDistanceBuffer{distance}
// {
//     gpio_set_direction(pin_trigger, GPIO_MODE_OUTPUT);
//     gpio_set_direction(pin_echo, GPIO_MODE_INPUT);
// }

DistanceBuffer::DistanceBuffer()
    : IDistance(), mDistanceBuffer{}
{
}

void DistanceBuffer::recordDistance(int32_t distance)
{
    mDistanceBuffer.push_back(distance);
}

void DistanceBuffer::setTriggerPin(gpio_num_t trigger) {
    this->pin_trigger = trigger;
    gpio_set_direction(this->pin_trigger, GPIO_MODE_OUTPUT);
    gpio_set_level(this->pin_trigger, 0);
}

void DistanceBuffer::setEchoPin(gpio_num_t echo) {
    this->pin_echo = echo;
    gpio_set_direction(this->pin_echo, GPIO_MODE_INPUT);
    gpio_set_level(this->pin_echo, 0);
}

// void IRAM_ATTR echoHandler(void* arg) // interrupt compatible implementation to read the value of the echo pin, passed by reference to interrupt allocator to explicitly pass argument to esp_intr_alloc
// {
//     ESP_LOGI(name, "Hello from distance buffer");
// }

float DistanceBuffer::getDistance()
{

    int max_distance_cm_temp = 100;

    // int64_t start_time, end_time;
    // float measured_distance = -1; // getDistance in a try / catch block, error type is -Werror=return-type (return-type)

    // gpio_set_level(this->pin_trigger, 0); // ensure pin driven low in order to pulse sensor
    // vTaskDelay(1 / portTICK_PERIOD_MS); // delay to ensure clear pulse

    // gpio_set_level(this->pin_trigger, 1); 
    // vTaskDelay(0.01 / portTICK_PERIOD_MS); // 10 us pulse (not sure if vTaskDelay supports consistent microsecond delays), needs to be non-blocking though
    // gpio_set_level(this->pin_trigger, 0); // end pulse

    // intr_handle_t temp_handle;
    // esp_intr_alloc(gpio_get_level(this->pin_echo) /*pin needs to be set as gpio_intr_t*/, ESP_INTR_FLAG_SHARED /*lowest level interrupt priority*/, this->echoHandler(static_cast<void*>this->handler_timestamps), NULL, &temp_handle); // allocate interrupt

    // start_time = esp_timer_get_time(); // get start time of pulse
    // end_time = esp_timer_get_time(); // get end time of pulse

    // measured_distance = (start_time - end_time) * SPEED_SOUND / (2) / (1000000) * (1000); // (microseconds - microseconds) * meters / second / 10^6 (seconds) / 2 * 1000 (millimeters ) = millimeters or mm 

    // // TODO: needs better data types later on

    // return measured_distance;

}

void DistanceBuffer::clearBuffer()
{
    mDistanceBuffer.clear();
}
