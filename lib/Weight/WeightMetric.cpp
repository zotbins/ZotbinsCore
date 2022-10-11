#include "WeightMetric.hpp"
#include "hx711.h"
using namespace Weight;

WeightMetric::WeightMetric() 
: mWeightSensor{{
        .dout = GPIO_NUM_19,
        .pd_sck = GPIO_NUM_18,
        .gain = HX711_GAIN_A_64
    }}
{
    /*
    hx711_t dev = {
        .dout = GPIO_NUM_19,
        .pd_sck = GPIO_NUM_18,
        .gain = HX711_GAIN_A_64
    };

    ESP_ERROR_CHECK(hx711_init(&dev));

    mWeightSensor = dev;
    */
}

int32_t WeightMetric::getWeight()
{
    int32_t data = 0;
    r = hx711_read_average(&mWeightSensor, 10, &data);
    if (r == ESP_OK) {
        return data;
    }

    return 0;
}
