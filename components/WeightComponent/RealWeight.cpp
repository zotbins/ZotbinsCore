#include "include/RealWeight.hpp"

using namespace Weight;

RealWeight::RealWeight()
{
    hx711_t devs[4] = {
        { .dout = GPIO_NUM_19, .pd_sck = GPIO_NUM_18, .gain = HX711_GAIN_A_128 },
        { .dout = GPIO_NUM_21, .pd_sck = GPIO_NUM_22, .gain = HX711_GAIN_A_128 },
        { .dout = GPIO_NUM_23, .pd_sck = GPIO_NUM_25, .gain = HX711_GAIN_A_128 },
        { .dout = GPIO_NUM_26, .pd_sck = GPIO_NUM_27, .gain = HX711_GAIN_A_128 }
    };

    for (int i = 0; i < 4; ++i) {
        mRealSensor[i] = devs[i];
        hx711_init(&mRealSensor[i]);
    }
}

int32_t RealWeight::getWeight()
{
    int32_t data[4] = {0};
    int32_t totalWeight = 0;

    for (int i = 0; i < 4; ++i) {
        esp_err_t r = hx711_read_average(&mRealSensor[i], 10, &data[i]);
        if (r == ESP_OK) {
            totalWeight += data[i];
        }
    }

    return totalWeight / 4; // avg weight
}