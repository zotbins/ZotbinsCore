#include "RealWeight.hpp"

using namespace Weight;

RealWeight::RealWeight()
{
    hx711_t dev = {
        .dout = GPIO_NUM_19,
        .pd_sck = GPIO_NUM_18,
        .gain = HX711_GAIN_A_64
    };

    mRealSensor = dev;

}

int32_t RealWeight::getWeight()
{
    int32_t data = 0;
    esp_err_t r = hx711_read_average(&mRealSensor, 10, &data);
    if (r == ESP_OK) {
        return data;
    }

    return 0;
}