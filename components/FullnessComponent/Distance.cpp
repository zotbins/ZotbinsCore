#include "Distance.hpp"
#include "esp_log.h"
#include "ultrasonic.h"
#include <driver/gpio.h>

using namespace Fullness;

static const char *name = "distance";

Distance::Distance(gpio_num_t trigger, gpio_num_t echo) : sensor{trigger, echo}
{
    ultrasonic_init(&this->sensor);
}

float Distance::getDistance()
{
    float measured_distance = -1; // if return -1 did not read properly, handle accordingly - TODO replace with error type

    esp_err_t res = ultrasonic_measure(&sensor, this->max_distance_cm, &measured_distance);
    if (res != ESP_OK)
    {
        ESP_LOGE(name, "Error %d: ", res);
        switch (res)
        {
        case ESP_ERR_ULTRASONIC_PING:
            ESP_LOGE(name, "Cannot ping (device is in invalid state)\n");
            break;
        case ESP_ERR_ULTRASONIC_PING_TIMEOUT:
            ESP_LOGE(name, "Ping timeout (no device found)\n");
            break;
        case ESP_ERR_ULTRASONIC_ECHO_TIMEOUT:
            ESP_LOGE(name, "Echo timeout (i.e. distance too big)\n");
            break;
        default:
            ESP_LOGE(name, "%s\n", esp_err_to_name(res));
        }
    }
    else
    {
        // ESP_LOGI(name, "Distance: %0.04f cm\n", measured_distance * 100);
    }

    return measured_distance;
}
