#include "Weight.hpp"

Weight::Weight::Weight(IWeight &weightSensor)
    : mWeightSensor{weightSensor}
{
}

int32_t Weight::Weight::getWeight()
{
    return mWeightSensor.getWeight();
}
