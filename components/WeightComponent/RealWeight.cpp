#include "RealWeight.hpp"
#include <cmath>
using namespace Weight;

RealWeight::RealWeight()
{
    hx711_t dev = {
        .dout = GPIO_NUM_19,
        .pd_sck = GPIO_NUM_18,
        .gain = HX711_GAIN_A_128
    };

    mRealSensor = dev;

    hx711_init(&mRealSensor);
}

// 1.4kg = 27550 in raw data.
// 1kg = 19680 in raw data.


int32_t RealWeight::getWeight()
{
    int32_t data = 0;
    esp_err_t r = hx711_read_average(&mRealSensor, 10, &data);
    if (r == ESP_OK) {
        return data;
    }

    return 0;
}