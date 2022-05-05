#include "MockWeight.hpp"

using namespace Weight;

MockWeight::MockWeight(int32_t weight)
    : mWeight{weight}
{
}

int32_t MockWeight::getWeight()
{
    return mWeight;
}