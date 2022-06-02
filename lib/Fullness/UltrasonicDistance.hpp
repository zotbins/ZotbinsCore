/**
 * @file MockDistance.hpp
 * @brief Mock distance class that inherits from IDistance interface class. Used for unit testing.
 * @version 0.1
 * @date 2022-05-11
 */

#include "IDistance.hpp"
#include <array>
#include <cstdint>
#include <stdlib.h>
#include <vector>

namespace Fullness
{
    class UltrasonicDistance final : public IDistance
    {
    public:
        /**
         * @brief Returns the distance from the mock sensor
         * @param int32_t Distance in millimeters
         *
         */
        UltrasonicDistance(int32_t trig, int32_t echo, size_t distanceIdx);

        /**
         * @brief returns distance
         * @return int32_t mDistanceBuffer
         */
        int32_t getDistance() override;

        /**
         * @brief returns distance
         * @return int32_t mDistanceBuffer
         */
        int32_t startDistance();

    private:
        int32_t trigPin;
        int32_t echoPin;
        size_t mDistanceBufferIdx;
        std::array<int32_t, 8> distances{};
    };
}