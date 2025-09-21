#ifndef PERIPHERAL_MANAGER_HPP
#define PERIPHERAL_MANAGER_HPP

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

void init_manager(void);
static void run_manager(void *arg);
static void publish_payload_temp(float fullness, float weight, int usage);

#endif // PERIPHERAL_MANAGER_HPP