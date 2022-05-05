#include "MockDistance.hpp"

MockDistance::MockDistance(int32_t distance)
    : mDistance{distance}
{
}

int32_t MockDistance::getDistance()
{
    return mDistance;
}