/**
 * @file CameraTask.hpp
 * @brief Header file for gpsTask class
 * @version 0.1
 * @date 2025-02-17
 *
 */

#ifndef GPS_TASK_HPP
#define GPS_TASK_HPP

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "task.hpp"

namespace Zotbins
{
    /**
     * @brief Task to measure the camera rate of the bin using the breakbeam sensor
     *
     */
    class GpsTask : public Task
    {
    public:
        /**
         * @brief Construct a new Camera Task object
         *
         * @param messageQueue Message queue to WiFi Task
         */
        explicit GpsTask(QueueHandle_t &messageQueue);

        /**
         * @brief Start execution of Camera Task
         *
         */
        void start() override;

    private:
        /**
         * @brief Function to be called by FreeRTOS function xTaskCreate().
         * Calls setup() and loop()
         *
         * @param task Camera Task objet casted as void pointer
         */
        static void taskFunction(void *task);

        /**
         * @brief Setup Camera Task before looping forever.
         * This function should not be called outside of taskFunction()
         *
         */
        void setup();

        /**
         * @brief Loop inside of Camera Task.
         * This function should not be called outside of taskFunction().
         *
         */
        void loop();

        /**
         * @brief Message queue to WiFi task
         *
         */
        QueueHandle_t &mMessageQueue;
        // gpio_num_t DETECT_PIN = GPIO_NUM_16;
    };
}

#endif