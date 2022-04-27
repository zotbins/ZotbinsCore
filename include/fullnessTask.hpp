/**
 * @file fullnessTask.hpp
 * @brief Header file for Fullness Task for Zotbins Core
 * @version 0.1
 * @date 2022-04-25
 * 
 */

#ifndef FULLNESS_TASK_HPP
#define FULLNESS_TASK_HPP

#include "task.hpp"

namespace Zotbins
{
    /**
     * @brief Task to measure fullness of the bin by using the ultrasonic sensor
     *
     */
    class FullnessTask : public Task
    {
    public:
        /**
         * @brief Construct a new Fullness Task object
         *
         */
        FullnessTask();

        /**
         * @brief Start execution of Fullness Task
         *
         */
        void start() override;

    private:
        /**
         * @brief Function to be called by FreeRTOS function xTaskCreate().
         * Calls setup() and loop()
         *
         * @param task Fullness Task object casted as void pointer
         */
        static void taskFunction(void *task);

        /**
         * @brief Setup Fullness Task before looping forever.
         * This function should not be called outside of taskFunction()
         *
         */
        void setup();

        /**
         * @brief Loop inside of Fullness Task.
         * This function should not be called outside of taskFunction().
         *
         */
        void loop();
    };
}

#endif