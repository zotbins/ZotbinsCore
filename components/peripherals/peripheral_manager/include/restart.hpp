#ifndef RESTART_HPP
#define RESTART_HPP
#include "freertos/FreeRTOS.h" 
#include "freertos/event_groups.h"
#include "esp_err.h"
#include "esp_system.h"
#include "esp_system.h"


#ifdef __cplusplus
extern "C" {
#endif

extern EventGroupHandle_t restartGroup;


#define SYSTEM_FLAG  (1 << 0)
#define SERVER_FLAG  (1 << 0)
/**
 * @breif Restart Task waits for two bits to be set (Server Bit and System Bit) before Restarting ESP32
 */
void restartTask(EventGroupHandle_t restartGroup);
/**
 * @breif Sets System Bit
 */
void updateRestart(int requirement, EventGroupHandle_t restartGroup);
/**
 * Sends message to server and restarts ESP32
 */
void restart();
/**
 * Sets Server Bit
 */
void receiveServer(EventGroupHandle_t restartGroup);

#ifdef __cplusplus
} 
#endif

#endif