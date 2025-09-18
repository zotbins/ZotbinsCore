#include "freertos/event_groups.h"

// RTOS event group for system initialization, i.e. wait for WiFi connection, MQTT connection, etc.
EventGroupHandle_t sys_init_eg;