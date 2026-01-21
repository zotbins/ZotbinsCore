#ifndef STATUS_HPP
#define STATUS_HPP
#include "esp_err.h"

/**
 * @brief Start's Timer for the Bin
 */
esp_err_t startTimer();

/**
 * @brief Sends message to AWS server every 5 minutes
 */
void sendMessage();

#endif