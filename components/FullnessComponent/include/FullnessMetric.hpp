/**
 * @file FullnessMetric.hpp
 * @brief Header file for Fullness class for an hcsr04
 * @version 0.1
 * @date 2022-05-04
 *
 */

#ifndef FULLNESS_HPP
#define FULLNESS_HPP

#include "Distance.hpp"
#include <array>
#include <cstddef>
#include <cstdint>

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
        FullnessMetric(uint32_t binHeight, Distance &distanceSensor);

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

        float IQM(std::array<int32_t, mDistanceBufferSize> &distanceBuffer); // interquartile mean, a statistical "average" across multiple values. i assume iqm uses shell sort to sort the values, then creates the iqm using standard formulas, then passes that into (is called by) getFullness to calculate a final percentage value. otherwise the iqm could be of fullness values but that would likely be slower requiring multiple calls to getFullness

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
        Distance &mDistanceSensor;
    };
}
#endif