#ifndef CLIENT_SERIALIZE_HPP
#define CLIENT_SERIALIZE_HPP

#include <cJSON.h>
#include <stdint.h>

namespace Client
{
    /*
     * @brief Serialize data from sensors for API interaction.
     *
     * The current shape of the JSON object returned is the following:
     * {
     *   "bin_id": 18838586676582,
     *   "fullness": 0.5,
     *   "usage": 44,
     *   "overflow": false,
     *   "weight": 50
     * }
     *
     * @return cJSON* root of a JSON object containing sensor data. Make sure to free this after you are done using it.
     */
    cJSON *serialize(float fullness, uint32_t usage, bool overflow, int32_t weight);
}

#endif
