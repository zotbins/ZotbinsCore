/**
 * @file ServoTask.hpp
 * @brief Header file for servo Task for Zotbins Core
 * @version 0.1
 * @date 2022-04-25
 *
 */

#ifndef SERVO_TASK_HPP
#define SERVO_TASK_HPP

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "task.hpp"

namespace Zotbins
{
    /**
     * @brief Task to measure servo of the bin by using the distance sensor
     *
     */
    class ServoTask : public Task
    {
    public:
        /**
         * @brief Construct a new servo Task object
         *
         */
        explicit ServoTask(QueueHandle_t &messageQueue);

        /**
         * @brief Start execution of servo Task
         *
         */
        void rotate();
        void start() override;

    private:
        /**
         * @brief Function to be called by FreeRTOS function xTaskCreate().
         * Calls setup() and loop()
         *
         * @param task servo Task object casted as void pointer
         */
        static void taskFunction(void *task);

        /**
         * @brief Setup servo Task before looping forever.
         * This function should not be called outside of taskFunction()
         *
         */
        void setup();

        /**
         * @brief Loop inside of servo Task.
         * This function should not be called outside of taskFunction().
         *
         */
        void loop();

        /**
         * @brief Message queue to WiFi Task
         *
         */

        QueueHandle_t &mMessageQueue;
    };
}

#endif