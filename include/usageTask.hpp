/**
 * @file usageTask.hpp
 * @brief Header file for UsageTask class
 * @version 0.1
 * @date 2022-04-27
 *
 */

#ifndef USAGE_TASK_HPP
#define USAGE_TASK_HPP

#include "task.hpp"

namespace Zotbins
{
    /**
     * @brief Task to measure the usage rate of the bin using the breakbeam sensor
     *
     */
    class UsageTask : public Task
    {
    public:
        /**
         * @brief Construct a new Usage Task object
         *
         */
        UsageTask();

        /**
         * @brief Start execution of Usage Task
         *
         */
        void start() override;

    private:
        /**
         * @brief Function to be called by FreeRTOS function xTaskCreate().
         * Calls setup() and loop()
         *
         * @param task Usage Task objet casted as void pointer
         */
        static void taskFunction(void *task);

        /**
         * @brief Setup Usage Task before looping forever.
         * This function should not be called outside of taskFunction()
         *
         */
        void setup();

        /**
         * @brief Loop inside of Usage Task.
         * This function should not be called outside of taskFunction().
         *
         */
        void loop();
    };
}

#endif