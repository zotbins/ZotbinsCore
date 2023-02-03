/**
 * @file message.hpp
 * @brief Header file for Message struct
 * @version 0.1
 * @date 2022-05-07
 *
 *
 */

#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <cstdint>

namespace Zotbins
{
    /**
     * @brief Message type.
     * Indicates origin of message.
     *
     */
    enum class MessageType
    {
        FullnessMessage,
        UsageMessage,
        WeightMessage
    };

    /**
     * @brief Data payload of message
     *
     */
    union MessagePayload
    {
        /**
         * @brief int data payload
         *
         */
        int32_t intPayload;

        /**
         * @brief float data payload
         *
         */
        float floatPayload;
    };

    /**
     * @brief Message for WiFi message queue
     *
     */
    struct Message
    {
        /**
         * @brief Message type.
         * Indicates origin of message.
         */
        MessageType msgType;

        /**
         * @brief Data payload of message
         *
         */
        MessagePayload msgPayload;
    };
}

#endif