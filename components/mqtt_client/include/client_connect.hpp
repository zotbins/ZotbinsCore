#ifndef CLIENT_CONNECT_HPP
#define CLIENT_CONNECT_HPP

#include "mqtt_client.h"

/**
 * @brief Connects the MQTT client to the broker using the credentials provided in the folder mqtt_client/credentials/ --- highly derived from esp-idf example code.
 * 
 */
void client_connect(void);

/**
 * @brief Disconnect from the MQTT broker and destroy the client object.
 * 
 */
void client_disconnect(void);

/**
 * @brief Returns the MQTT client handle. Used to publish messages to the active client without fatal errors.
 * 
 * @return esp_mqtt_client_handle_t 
 */
esp_mqtt_client_handle_t get_client_handle(void);

#endif // CLIENT_CONNECT_HPP