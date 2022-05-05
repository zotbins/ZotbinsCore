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
#include <cstdint>

namespace Zotbins
{
    class Fullness
    {
        /**
         * @brief The fullness class
         *
         * @param height Name of task
         * @param distance Priority number of task
         */

    public:
        /*constructor*/
        // Fullness::Fullness(int32_t binHeight, IDistance::IDistance &distanceSensor)
        //     : height(binHeight), sensor(&distanceSensor) // FIGURE THIS OUT LATER PLS
        // {
        // }

        Fullness(int32_t binHeight, IDistance::IDistance &distanceSensor);

        float getFullness();
        bool isValidFullness();

    private:
        /**
         * @brief bin height
         *
         */
        int32_t height;

        /**
         * @brief bin height of trash to top
         *
         */
        IDistance::IDistance *sensor;
    };
}
#endif