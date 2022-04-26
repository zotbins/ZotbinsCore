#ifndef IDISTANCE_HPP
#define IDISTANCE_HPP

#include <cstdint>

/**
 * @brief Interface (abstract) class for mock and real hardware ultrasonic (fullness/distance) sensor.
 * Used to inherit the getDistance method in derived classes.
 *
 */
class IDistance
{
    /**
     * @brief Returns the distance of the sensor.
     *
     * @return int32_t
     */
    virtual int32_t getDistance() = 0;
};

#endif