#include "Weight.hpp"

Weight::Weight(IWeight &weightSensor)
    : m_weightSensor{&weightSensor}
{
}

int32_t Weight::getWeight()
{
    return m_weightSensor->getWeight();
}
