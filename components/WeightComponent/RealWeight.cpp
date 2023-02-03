#include "RealWeight.hpp"

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

int32_t RealWeight::getWeight()
{
    int32_t data = 0;
    esp_err_t r = hx711_read_average(&mRealSensor, 10, &data);
    //esp_err_t r = hx711_read_data(&mRealSensor, &data);
    if (r == ESP_OK) {
        return data;
    }

    return 0;
}