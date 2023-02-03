#include "WeightMetric.hpp"

using namespace Weight;

WeightMetric::WeightMetric(IWeight &weightSensor)
    : mWeightSensor{weightSensor}
{
}

int32_t WeightMetric::getWeight()
{
    return mWeightSensor.getWeight();
}
    