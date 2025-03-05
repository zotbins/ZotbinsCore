/**
 * @file Distance.hpp
 * @brief Header file for ultrasonic distance class
 * @version 0.1
 * @date 2022-04-27
 *
 */

#ifndef DISTANCE_HPP
#define DISTANCE_HPP

#include "ultrasonic.h"
#include <cstdint>
#include <driver/gpio.h>
#include <esp_intr_alloc.h>
#include <freertos/FreeRTOS.h>

namespace Fullness
{
    /**
     * @brief
     */
    class Distance
    {

    public:
        // /**
        //  * @brief Construct a new Distance object (init with trigger and echo)
        //  */
        // Distance(gpio_num_t trigger, gpio_num_t echo, float max_distance_cm); // TODO: NEED TO OVERLOAD CONSTRUCTOR TO SUPPORT MAX_DISTANCE

        /**
         * @brief Construct a new Distance object (init with trigger and echo)
         */
        Distance(gpio_num_t trigger, gpio_num_t echo);

        /**
         * @brief Choose the max distance that the ultrasonic sensor will measure, such as the height of the bin.
         */        
        void setMaxDistance(int max_distance_cm);

        /**
         * @brief Returns the distance of the sensor.
         * @return int32_t Distance in millimeters
         */
        float getDistance();

    private:
        const ultrasonic_sensor_t sensor;
        float max_distance_cm;
    };
}

#endif