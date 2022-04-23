#ifndef IWEIGHT_HPP
#define IWEIGHT_HPP

#include <cstdint>

class IWeight 
{
    /**
     * @brief Returns the weight of the sensor.
     * 
     * @return int32_t 
     */

    virtual int32_t getWeight() = 0;
};

#endif