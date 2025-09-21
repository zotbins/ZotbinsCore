#ifndef CLIENT_PUBLISH_HPP
#define CLIENT_PUBLISH_HPP

/**
 * @brief Publishes data to the MQTT broker on topic "binData"
 *
 * @param data
 */
void client_publish(const char *data);

#endif // CLIENT_PUBLISH_HPP