/**
 * @file fullnessTask.hpp
 * @brief Header file for Fullness Task for Zotbins Core
 * @version 0.1
 * @date 2022-04-25
 *
 */

#ifndef FULLNESS_TASK_HPP
#define FULLNESS_TASK_HPP

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "task.hpp"

#include "DistanceBuffer.hpp"
#include "FullnessMetric.hpp"
#include "esp_log.h"
#include <driver/gpio.h>


namespace Zotbins
{
    class FullnessTask : public Task
    {
    public:
        /**
         * @brief Construct a new Fullness Task object
         *
         */
        explicit FullnessTask(QueueHandle_t &messageQueue);

        /**
         * @brief Start execution of Fullness Task
         *
         */
        void start() override;

        float getFullness();
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

        /**
         * @brief Message queue to WiFi Task
         *
         */
        QueueHandle_t &mMessageQueue;

        /**
         * @brief Distance sensor for measuring bin fullness
         *
         */
        Fullness::Distance ultrasonic;
        
        /**
         * @brief Variable to store distance measurements
         *
         */
        float distance;

    };
}

#endif