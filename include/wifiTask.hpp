/**
 * @file wifiTask.hpp
 * @brief Header file for WifiTask class
 * @version 0.1
 * @date 2022-04-27
 *
 */

#ifndef WIFI_TASK_HPP
#define WIFI_TASK_HPP

#include "task.hpp"

namespace Zotbins
{
    /**
     * @brief Task to send sensor data to MQTT broker
     * 
     */
    class WiFiTask : public Task
    {
    public:
        /**
         * @brief Construct a new Wi Fi Task object
         *
         */
        WiFiTask();

        /**
         * @brief Start execution of WiFi Task
         *
         */
        void start() override;

    private:
        /**
         * @brief Function to be called by FreeRTOS function xTaskCreate().
         * Calls setup() and loop()
         *
         * @param task WiFi Task object casted as void pointer
         */
        static void taskFunction(void *task);

        /**
         * @brief Setup WiFi Task before looping forever.
         * This function should not be called outside of taskFunction()
         *
         */
        void setup();

        /**
         * @brief Loop inside of WiFi Task.
         * This function should not be called outside of taskFunction().
         *
         */
        void loop();

        /**
         * @brief Connect to WiFi for first time
         *
         * @return true/false if successfully connected to WiFi or not
         */
        bool setupWifi();
    };

}

#endif