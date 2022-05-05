#include "Weight.hpp"

Weight::Weight::Weight(IWeight& weightSensor)
    : m_weightSensor{weightSensor}
{
}

int32_t Weight::Weight::getWeight()
{
    return m_weightSensor.getWeight();
}
