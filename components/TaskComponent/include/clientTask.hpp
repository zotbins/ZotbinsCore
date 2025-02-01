/**
 * @file clientTask.hpp
 * @brief Header file for Client class
 * @version 0.1
 * @date 2024-04-08
 *
 */

#ifndef CLIENT_TASK_HPP
#define CLIENT_TASK_HPP

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "task.hpp"

namespace Zotbins
{
    /**
     * @brief Task to measure fullness of the bin by using the distance sensor
     *
     */
    class ClientTask : public Task
    {
    public:
        /**
         * @brief Construct a new MQTT Client Task object
         *
         */
        explicit ClientTask(QueueHandle_t &messageQueue);

        /**
         * @brief Start execution of Client Task
         *
         */
        void start() override;

    private:
        /**
         * @brief Function to be called by FreeRTOS function xTaskCreate().
         * Calls setup() and loop()
         *
         * @param task Client Task object casted as void pointer
         */
        static void taskFunction(void *task);

        /**
         * @brief Setup Client Task before looping forever.
         * This function should not be called outside of taskFunction()
         *
         */
        void setup();

        /**
         * @brief Loop inside of Client Task.
         * This function should not be called outside of taskFunction().
         *
         */
        void loop();
    };
}

#endif