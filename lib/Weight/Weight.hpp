/**
 * @file Weight.hpp
 * @brief Header file for Weight class
 * @version 0.1
 * @date 2022-05-05
 *
 */

#ifndef WEIGHT_HPP
#define WEIGHT_HPP
#include "IWeight.hpp"

namespace Weight
{
    /**
     * @brief Acts as main class, taking in weight interface to allow for mock data and real data
     *
     */
    class Weight
    {
    public:
        /**
         * @brief Construct a new Weight object.
         * @param weightSensor An object that is a weight interface object.
         *
         */
        Weight(IWeight &weightSensor);

        /**
         * @brief Returns the weight.
         * @return int32_t weight in kilograms
         * 
         */
        int32_t getWeight();

    private:
        /**
         * @brief IWeight object, uses pointer because virtual function
         *
         */
        IWeight &mWeightSensor;
    };
}

#endif