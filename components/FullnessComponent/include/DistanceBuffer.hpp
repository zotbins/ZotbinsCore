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
#include <driver/gpio.h>

namespace Fullness
{
    class DistanceBuffer final : public IDistance
    {
    public:

        /**
         * @brief Buffer containing a window of distance readings from the sensor
         * @param int32_t Distance in millimeters
         *
         */
        // DistanceBuffer(gpio_num_t trigger, gpio_num_t echo, std::vector<int32_t> distance);

        DistanceBuffer();

        void setTriggerPin(gpio_num_t trigger) override;

        void setEchoPin(gpio_num_t echo) override;

        // void IRAM_ATTR echoHandler(void* arg) override;

        /**
         * @brief Returns distance
         * @return float mDistanceBuffer
         */
        float getDistance() override;

        /**
         * @brief Pushes a distance value to the buffer
         * @param int32_t Distance in millimeters
         */
        void recordDistance(int32_t distance);

        /**
         * @brief Clears the distance buffer
         */   
        void clearBuffer();

    private:

    };
}