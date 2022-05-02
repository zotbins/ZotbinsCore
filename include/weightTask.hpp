/**
 * @file weightTask.hpp
 * @brief Header file for WeightTask class
 * @version 0.1
 * @date 2022-04-27
 *
 */

#ifndef WEIGHT_TASK_HPP
#define WEIGHT_TASK_HPP

#include "task.hpp"

namespace Zotbins
{
    /**
     * @brief Task to measure current weight of the trash inside the bin
     * using the HX711 and load cell
     *
     */
    class WeightTask : public Task
    {
    public:
        /**
         * @brief Construct a new Weight Task object
         *
         */
        WeightTask();

        /**
         * @brief Start execution of Weight Task
         *
         */
        void start() override;

    private:
        /**
         * @brief Function to be called by FreeRTOS function xTaskCreate().
         * Calls setup() and loop()
         *
         * @param task Weight Task object casted as void pointer
         */
        static void taskFunction(void *task);

        /**
         * @brief Setup Weight Task before looping forever.
         * This function should not be called outside of taskFunction()
         *
         */
        void setup();

        /**
         * @brief Loop inside of Weight Task.
         * This function should not be called outside of taskFunction().
         *
         */
        void loop();
    };
}

#endif