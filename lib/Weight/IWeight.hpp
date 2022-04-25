#ifndef IWEIGHT_HPP
#define IWEIGHT_HPP

#include <cstdint>

/**
 * @brief Interface (abstract) class for mock and real hardware weight sensor.
 * Used to inherit the getWeight method in derived classes.
 * 
 */
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