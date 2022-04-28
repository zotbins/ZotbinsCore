#include "MockWeight.hpp"

MockWeight::MockWeight(int32_t mWeight)
    : mWeight{mWeight}
{
}

int32_t MockWeight::getWeight()
{
    return mWeight;
}