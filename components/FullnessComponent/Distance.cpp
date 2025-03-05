#include "Distance.hpp"
#include "esp_log.h"
#include "ultrasonic.h"
#include <driver/gpio.h>

using namespace Fullness;

static const char *name = "Distance";

Distance::Distance(gpio_num_t trigger, gpio_num_t echo) : sensor{trigger, echo}
{
    ultrasonic_init(&this->sensor);
}

void Distance::setMaxDistance(int max_distance_cm) {
    this->max_distance_cm = max_distance_cm;
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
            ESP_LOGE(name, "Cannot ping (device is in invalid state)");
            break;
        case ESP_ERR_ULTRASONIC_PING_TIMEOUT:
            ESP_LOGE(name, "Ping timeout (no device found)");
            break;
        case ESP_ERR_ULTRASONIC_ECHO_TIMEOUT:
            ESP_LOGE(name, "Echo timeout (i.e. distance too big)");
            break;
        default:
            ESP_LOGE(name, "%s", esp_err_to_name(res));
        }
    }
    else {
    }

    return measured_distance;
}
