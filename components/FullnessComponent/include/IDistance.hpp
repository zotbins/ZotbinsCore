/**
 * @file IDistance.hpp
 * @brief Header file for IDistance class (distance interface for the ultrasonic sensor)
 * @version 0.1
 * @date 2022-04-27
 *
 */

#ifndef IDISTANCE_HPP
#define IDISTANCE_HPP

#include <cstdint>
#include <driver/gpio.h>
#include <esp_intr_alloc.h>
#include <freertos/FreeRTOS.h>
#include <vector>


namespace Fullness
{
    /**
     * @brief Interface (abstract) class for mock and real hardware (fullness/distance) sensor.
     * Used to inherit the getDistance method in derived classes.
     *
     */
    class IDistance
    {
    public:
        // /**
        //  * @brief Construct a new IDistance object (set trigger and echo later)
        //  */
        // IDistance();

        // /**
        //  * @brief Construct a new IDistance object
        //  *
        //  * @param trigger trigger pin of the  
        //  * @param distanceSensor Interface to distance sensor
        //  */
        // IDistance(gpio_num_t trigger, gpio_num_t echo);

        /**
         * @brief Set trigger pin
         */
        virtual void setTriggerPin(gpio_num_t trigger) = 0;
        /**

         * @brief Set echo pin
         */
        virtual void setEchoPin(gpio_num_t echo) = 0;

        /**

         * @brief empty handler for interrupt / ultrasonic timing
         */
        // virtual void IRAM_ATTR echoHandler(void* arg) = 0;

        /**
         * @brief Returns the distance of the sensor.
         *
         * @return int32_t Distance in millimeters
         */
        virtual float getDistance() = 0; // = 0, pure specifier - defined in the derived class (distance buffer)

    protected:
        gpio_num_t pin_trigger;
        gpio_num_t pin_echo;
        std::vector<int64_t> handler_timestamps;

    };
}

#endif