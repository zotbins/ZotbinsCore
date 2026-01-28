#ifndef REMOTE_CONFIG_HPP
#define REMOTE_CONFIG_HPP

#include <esp_err.h>
#include <stddef.h>



/**
 * @brief The function takes in the “key” and “value” from the parseCommand function and sets in NVS config
 */
esp_err_t setNVS(const char* value, const char* key);

/**
 * @brief This function parses values from JSON command.
 */
void parseCommand(const char* command);

/**
 * @brief Gets values from the NVS storage - not used currently
 */
esp_err_t get_value_nvs(const char* key, char* value, size_t size);

/**
 * @brief Sends a JSON based payload to the AWS Server
 */
void sendSignal(const char* payload);


#endif // REMOTE_CONFIG_HPP