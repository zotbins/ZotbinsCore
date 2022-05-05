#include "IDistance.hpp"

/**
 * @brief Mock distance class that inherits from the IDistance interface class.
 * Used for unit tests.
 */
class MockDistance final : public Fullness::IDistance
{
public:
    /**
     * @brief Constructs an instance of the MockDistance class.
     * @param mDistance Current distance of mock sensor.
     *
     */
    MockDistance(int32_t distance);

    /**
     * @brief returns distance
     * @return int32_t
     */
    int32_t getDistance() override;

private:
    int32_t mDistance;
};