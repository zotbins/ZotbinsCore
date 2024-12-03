/**
 * @file RealWeight.hpp
 * @brief Header file for RealWeight class
 * @version 0.1
 * @date 2022-10-11
 *
 */

#ifndef REAL_WEIGHT_HPP
#define REAL_WEIGHT_HPP

#include "IWeight.hpp"
#include "hx711.h"

namespace Weight
{
    /**
     * @brief Real weight class that inherits from the IWeight interface class.
     * Wrapper class for the HX711.
     */
    class RealWeight final : public IWeight
    {
    public:
        /**
         * @brief Constructs an instance of the RealWeight class.
         *
         */
        RealWeight();

        /**
         * @brief Returns the weight.
         * @return int32_t Weight in kilograms.
         * 
         */
        int32_t getWeight() override;

    private:
        /**
         * @brief HX711 struct.
         *
         */
        hx711_t mRealSensor[4];
    };
}
#endif