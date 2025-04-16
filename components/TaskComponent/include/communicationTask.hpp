/**
 * @file CameraTask.hpp
 * @brief Header file for gpsTask class
 * @version 0.1
 * @date 2025-02-17
 *
 */

 #ifndef COMMUNICATION_TASK_HPP
 #define COMMUNICATION_TASK_HPP
 
 #include "task.hpp"
 #include "freertos/FreeRTOS.h"
 #include "freertos/queue.h"
 #include "freertos/task.h"
 #include "freertos/semphr.h"
 #include "driver/gpio.h"
 
 namespace Zotbins
 {
     /**
      * @brief Task to measure the camera rate of the bin using the breakbeam sensor
      *
      */
     class CommunicationTask : public Task
     {
     public:
         /**
          * @brief Construct a new Camera Task object
          *
          * @param messageQueue Message queue to WiFi Task
          */
         explicit CommunicationTask(QueueHandle_t &messageQueue);
 
         /**
          * @brief Start execution of Camera Task
          *
          */
         void start() override;
         static void sendMessage(const char *message);
 
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