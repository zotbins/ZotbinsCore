#ifndef CLIENT_SERIALIZE_HPP
#define CLIENT_SERIALIZE_HPP

#include <cJSON.h>
#include <stdint.h>

#if defined(CAMERA)
namespace Client
{
    /*
     * @brief Serialize data from the camera MCU for API interaction.
     *
     * JSON object format:
     * {
     *   "bin_id": 18838586676582,
     *   "mcu_type": "Camera",
     *   "message": string,
     *   "img_str": Image base64 string
     * }
     *
     * @return cJSON* root of a JSON object containing camera data. 
     *         Make sure to free this after use.
     */
    cJSON *serialize(char* message, char* img_str, size_t buffer_length, int compressedSize, int uncompressedSize);
}

#elif defined(SENSOR)
namespace Client
{
    /*
     * @brief Serialize data from the sensor MCU for API interaction.
     *
     * JSON object format:
     * {
     *   "bin_id": 18838586676582,
     *   "mcu_type": "Sensor",
     *   "message": string,
     *   "fullness": 0.5,
     *   "overflow": false,
     *   "weight": 50
     * }
     *
     * @return cJSON* root of a JSON object containing sensor data. 
     *         Make sure to free this after use.
     */
    cJSON *serialize(char* message, float fullness, bool overflow, int32_t weight, int usage);
}

#endif // MCU_TYPE

#endif // CLIENT_SERIALIZE_HPP
