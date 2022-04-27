/**
 * @file IWeight.hpp
 * @brief Header file for IWeight class
 * @version 0.1
 * @date 2022-04-25
 *
 */

#ifndef IWEIGHT_HPP
#define IWEIGHT_HPP

#include <cstdint>

namespace Weight
{
    /**
     * @brief Interface (abstract) class for mock and real hardware weight sensor.
     * Used to inherit the getWeight method in derived classes.
     *
     */
    class IWeight
    {
    public:
        /**
         * @brief Returns the weight of the sensor.
         *
         * @return int32_t Weight in kilograms
         */
        virtual int32_t getWeight() = 0;
    };
}

#endif