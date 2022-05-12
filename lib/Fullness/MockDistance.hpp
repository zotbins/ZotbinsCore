/**
 * @file MockDistance.hpp
 * @brief Mock distance class that inherits from IDistance interface class. Used for unit testing.
 * @version 0.1
 * @date 2022-05-11
 */

#include "IDistance.hpp"
#include <cstdint>
#include <stdlib.h>
#include <vector>

class MockDistance final : public Fullness::IDistance
{
public:
    /**
     * @brief Returns the distance from the mock sensor
     * @param int32_t Distance in millimeters
     *
     */
    MockDistance(std::vector<uint32_t> distance);

    /**
     * @brief returns distance
     * @return uint32_t
     */
    uint32_t getDistance() override;

private:
    std::vector<uint32_t> mDistanceBuffer;
    size_t mDistanceBufferIdx;
};