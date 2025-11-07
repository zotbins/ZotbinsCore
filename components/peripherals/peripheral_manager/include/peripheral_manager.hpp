#ifndef PERIPHERAL_MANAGER_HPP
#define PERIPHERAL_MANAGER_HPP

#define MANAGER_STATUS_EVENT_BIT BIT2
#define MANAGER_RUNNING 1
#define MANAGER_STOPPED 0

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/timers.h"

/**
 * @brief Initialize the peripheral manager task, which includes intializing its task, all peripherals, and the peripheral manager event group.
 *
 */
void init_manager(void);

/**
 * @brief Used in init_manager as the task function.
 *
 * @param arg
 */
static void run_manager(void *arg);

/**
 * @brief Serializes and publishes input data.
 *
 * @param fullness
 * @param weight
 * @param usage
 */
static void publish_payload(float fullness, float weight, int usage);

#endif // PERIPHERAL_MANAGER_HPP