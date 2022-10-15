#include "FullnessMetric.hpp"
#include "IDistance.hpp"

using namespace Fullness;

FullnessMetric::FullnessMetric(uint32_t binHeight, IDistance &distanceSensor) : mBinHeight(binHeight), mDistanceSensor(distanceSensor)
{
}

float FullnessMetric::getFullness()
{
    // call getDistance on sensor mDistanceBufferSize times to get an array of distances
    std::array<int32_t, mDistanceBufferSize> distanceBuffer;
    for (size_t i = 0; i < distanceBuffer.size(); i++)
    {
        distanceBuffer[i] = mDistanceSensor.getDistance();
    }

    // use IQM to get average distance
    int averageDistance = IQM(distanceBuffer);

    float fullness = (static_cast<float>(mBinHeight) - averageDistance) / mBinHeight;

    return fullness;
}

bool FullnessMetric::isValidDistance(uint32_t distance)
{
    return (mBinHeight < distance) && (distance >= 0);
}

float FullnessMetric::IQM(std::array<int32_t, mDistanceBufferSize> &distanceBuffer)
{
    int32_t averageDistance = 0;

    shellSort(distanceBuffer);

    // find the average not including the first two and last two indexes
    for (size_t i = 2; i < mDistanceBufferSize - 2; i++)
    {
        averageDistance += distanceBuffer[i];
    }
    averageDistance /= (mDistanceBufferSize - 4);

    return averageDistance;
}

void FullnessMetric::shellSort(std::array<int32_t, mDistanceBufferSize> &arr)
{
    for (size_t gap = arr.size() / 2; gap > 0; gap /= 2)
    {
        for (size_t i = gap; i < arr.size(); i += 1)
        {
            int32_t temp = arr[i];

            size_t j;
            for (j = i; j >= gap && arr[j - gap] > temp; j -= gap)
            {
                arr[j] = arr[j - gap];
            }

            arr[j] = temp;
        }
    }
}