#include "MockDistance.hpp"
#include <vector>

MockDistance::MockDistance(std::vector<uint32_t> distance) : mDistanceBuffer{distance}, mDistanceBufferIdx{0}
{
}

uint32_t MockDistance::getDistance()
{
    uint32_t distance = mDistanceBuffer[mDistanceBufferIdx];
    if (mDistanceBufferIdx < mDistanceBuffer.size())
    {
        mDistanceBufferIdx++;
    }
    else
    {
        mDistanceBufferIdx = 0;
    }

    return distance;
}