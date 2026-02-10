#ifndef PERIPHERAL_MANAGER_HPP
#define PERIPHERAL_MANAGER_HPP

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