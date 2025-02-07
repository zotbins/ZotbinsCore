// /**
//  * @file MockDistance.hpp
//  * @brief Mock distance class that inherits from IDistance interface class. Used for unit testing.
//  * @version 0.1
//  * @date 2022-05-11
//  */

// #include "Distance.hpp"
// #include <cstdint>
// #include <stdlib.h>
// #include <vector>
// #include <driver/gpio.h>

// namespace Fullness
// {
//     class DistanceBuffer final : public Distance
//     {

//     public:
//         /**
//          * @brief Create buffer object containing a window of distance readings from the sensor
//          */
//         DistanceBuffer();

//         /**
//          * @brief Pushes a distance value to the buffer
//          * @param int32_t Distance in millimeters
//          */
//         void recordDistance(float distance);

//         /**
//          * @brief Clears the distance buffer
//          */
//         void clearBuffer();

//     private:
//         std::vector<int32_t> buffer;

//     };
// }