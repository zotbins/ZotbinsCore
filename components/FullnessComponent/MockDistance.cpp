#include "MockDistance.hpp"
#include <vector>

using namespace Fullness;

MockDistance::MockDistance(std::vector<int32_t> distance)
    : mDistanceBuffer{distance}, mDistanceBufferIdx{0}
{
}

int32_t MockDistance::getDistance()
{
    int32_t distance = mDistanceBuffer[mDistanceBufferIdx];
    mDistanceBufferIdx = (mDistanceBufferIdx + 1) % mDistanceBuffer.size();
    return distance;
}