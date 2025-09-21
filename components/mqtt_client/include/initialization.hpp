#ifndef INITIALIZATION_HPP
#define INITIALIZATION_HPP

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

// RTOS event group for system initialization, i.e. wait for WiFi connection, MQTT connection, etc.
extern EventGroupHandle_t sys_init_eg;

/**
 * @brief Initializes the system startup event group sys_init_eg. Should be included and used by anything that should not run before the MQTT broker is connected.
 * 
 */
void initialize(void);

#endif // INITIALIZATION_HPP