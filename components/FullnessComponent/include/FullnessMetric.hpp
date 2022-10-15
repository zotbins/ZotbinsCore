/**
 * @file FullnessMetric.hpp
 * @brief Header file for Fullness class
 * @version 0.1
 * @date 2022-05-04
 *
 */

#ifndef FULLNESS_HPP
#define FULLNESS_HPP

#include "IDistance.hpp"
#include <array>
#include <cstdint>
#include <cstddef>

namespace Fullness
{
    /**
     * @brief Measures the fullness of the waste bin
     *
     */
    class FullnessMetric
    {
    public:
        /**
         * @brief Construct a new FullnessMetric object
         *
         * @param binHeight Height of bin in mm
         * @param distanceSensor Interface to distance sensor
         */
        FullnessMetric(uint32_t binHeight, IDistance &distanceSensor);

        /**
         * @brief Returns the fullness percentage of the bin
         *
         * @return float Fullness % of the bin (0 <= fullness <= 1)
         */
        float getFullness();

        /**
         * @brief Checks if distance is valid
         *
         * @param distance Distance given from distance sensor
         * @return true/false if distance is valid
         */
        bool isValidDistance(uint32_t distance);

    private:
        static constexpr size_t mDistanceBufferSize = 8;

        float IQM(std::array<int32_t, mDistanceBufferSize> &distanceBuffer);

        void shellSort(std::array<int32_t, mDistanceBufferSize> &arr);

        /**
         * @brief Height of bin in mm
         *
         */
        uint32_t mBinHeight;

        /**
         * @brief Distance sensor
         *
         */
        IDistance &mDistanceSensor;
    };
}
#endif