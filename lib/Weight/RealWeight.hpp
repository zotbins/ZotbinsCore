/**
 * @file RealWeight.hpp
 * @brief Header file for RealWeight class
 * @version 0.1
 * @date 2022-05-13
 *
 */

#ifndef REAL_WEIGHT_HPP
#define REAL_WEIGHT_HPP

#include "IWeight.hpp"
#include "HX711.h"

namespace Weight
{
    /**
     * @brief Real weight class that inherits from the IWeight interface class.
     * 
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
         * @brief Returns the real weight.
         * @return int32_t Weight in kilograms.
         * 
         */
        int32_t getWeight() override;

    private:

        HX711 mSensor;
        const int32_t hx_out_pin = 16; // HARDCODED
        const int32_t hx_sck_pin = 4; // HARDCODED
        const int32_t mCalibrationFactor = 19.94; // HARDCODED FROM OUR TESTS, CAN REFINE THIS NUMBER LATER
    };
}
#endif