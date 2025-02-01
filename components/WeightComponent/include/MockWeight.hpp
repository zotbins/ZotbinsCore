/**
 * @file MockWeight.hpp
 * @brief Header file for MockWeight class
 * @version 0.1
 * @date 2022-05-05
 *
 */

#ifndef MOCK_WEIGHT_HPP
#define MOCK_WEIGHT_HPP

#include "IWeight.hpp"

namespace Weight
{
    /**
     * @brief Mock weight class that inherits from the IWeight interface class.
     * Used for unit tests so that actual hardware isn't require to test the ZotBins
     * system.
     */
    class MockWeight final : public IWeight
    {
    public:
        /**
         * @brief Constructs an instance of the MockWeight class.
         * @param weight Represents the current weight of the mock sensor.
         *
         */
        MockWeight(int32_t weight);

        /**
         * @brief Returns the mock weight.
         * @return int32_t Weight in kilograms.
         *
         */
        int32_t getWeight() override;

    private:
        /**
         * @brief Represents the current weight of the mock sensor.
         *
         */
        int32_t mWeight;
    };
}
#endif