/**
 * @file Fullness.hpp
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

namespace NFullness
{
    /**
     * @brief Measures the fullness of the waste bin
     *
     */
    class Fullness
    {
    public:
        /**
         * @brief Construct a new Fullness object
         *
         * @param binHeight
         * @param distanceSensor Ultrasonic sensor that measures distance
         */
        Fullness(uint32_t binHeight, IDistance &distanceSensor);

        /**
         * @brief Returns the fullness percentage of the bin
         *
         * @return float Fullness Percentage (0 <= fullness <= 1)
         */
        float getFullness();

        /**
         * @brief
         *
         * @param distance
         * @return true
         * @return false
         */
        bool isValidDistance(uint32_t distance);

    private:
        static constexpr std::size_t distanceBufferSize = 8;

        float IQM(std::array<int32_t, distanceBufferSize> &distanceBuffer);

        void shellSort(std::array<int32_t, distanceBufferSize> &arr);

        /**
         * @brief bin height
         *
         */
        uint32_t mBinHeight;

        /**
         * @brief bin height of trash to top
         *
         */
        IDistance &mDistanceSensor;
    };
}
#endif