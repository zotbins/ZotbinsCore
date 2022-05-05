/**
 * @file IDistance.hpp
 * @brief Header file for IDistance class
 * @version 0.1
 * @date 2022-04-27
 *
 */

#ifndef IDISTANCE_HPP
#define IDISTANCE_HPP

#include <cstdint>

namespace Fullness
{
    /**
     * @brief Interface (abstract) class for mock and real hardware ultrasonic (fullness/distance) sensor.
     * Used to inherit the getDistance method in derived classes.
     *
     */
    class IDistance
    {
    public:
        /**
         * @brief Returns the distance of the sensor.
         *
         * @return int32_t Distance in millimeters
         */
        virtual int32_t getDistance() = 0;
    };
}

#endif