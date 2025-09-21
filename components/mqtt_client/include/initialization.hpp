#ifndef INITIALIZATION_HPP
#define INITIALIZATION_HPP

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

// RTOS event group for system initialization, i.e. wait for WiFi connection, MQTT connection, etc.
extern EventGroupHandle_t sys_init_eg;
void initialize();

#endif // INITIALIZATION_HPP