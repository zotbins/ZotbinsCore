#include "Serialize.hpp"
#include <esp_mac.h>

// void buffer_to_string(const void *buffer, size_t buffer_length, char *output, size_t output_size)
// {
//     size_t pos = 0;
//     // pos += snprintf(output + pos, output_size - pos, "[");
//     for (size_t i = 0; i < buffer_length; i++)
//     {
//         if (i > 0)
//         {
//             pos += snprintf(output + pos, output_size - pos, ",");
//         }
//         pos += snprintf(output + pos, output_size - pos, "%u", buffer[i]);
//     }
//     // snprintf(output + pos, output_size - pos, "]");
// }

uint64_t getMAC(){
    uint8_t mac_addr_arr[6];
    uint64_t mac_addr;
    if (esp_base_mac_addr_get(mac_addr_arr) != ESP_OK){
        // TODO: Handle mac address being unset properly
        mac_addr = 0x112233445566;
    }
    else{
        mac_addr =
            ((uint64_t)mac_addr_arr[5] << 0) |
            ((uint64_t)mac_addr_arr[4] << 8) |
            ((uint64_t)mac_addr_arr[3] << 16) |
            ((uint64_t)mac_addr_arr[2] << 24) |
            ((uint64_t)mac_addr_arr[1] << 32) |
            ((uint64_t)mac_addr_arr[0] << 40);
    }
    return mac_addr;
}

// #if MCU_TYPE == CAMERA
// cJSON *Client::serializeInitImage(int photoID, int totalCount)
// {
//     // Use the ESP32's MAC address as a UUID (since it is a UUID)
//     uint64_t mac_addr = getMAC();
//     cJSON *root = cJSON_CreateObject();
//     cJSON_AddNumberToObject(root, "bin_id", mac_addr);
//     cJSON_AddNumberToObject(root, "photoID", photoID);
//     cJSON_AddNumberToObject(root, "totalCount", totalCount);
//     return root;
// }

//#if MCU_TYPE == CAMERA
// cJSON *Client::serializeSendImage(int photoID, int imageNumber, char* image)
// {
//     uint64_t mac_addr = getMAC();
//     cJSON *root = cJSON_CreateObject();
//     cJSON_AddNumberToObject(root, "bin_id", mac_addr);
//     cJSON_AddNumberToObject(root, "photoID", photoID);
//     cJSON_AddNumberToObject(root, "imageNumber", imageNumber);
//     cJSON_AddStringToObject(root, "image", image);
//     return root;
// }


// #if MCU_TYPE == SENSOR
cJSON *Client::serialize(char* message, float fullness, bool overflow, int32_t weight)
{
    // Use the ESP32's MAC address as a UUID (since it is a UUID)
    uint8_t mac_addr_arr[6];
    uint64_t mac_addr;
    if (esp_base_mac_addr_get(mac_addr_arr) != ESP_OK)
    {
        // TODO: Handle mac address being unset properly
        mac_addr = 0x112233445566;
    }
    else
    {
        mac_addr =
            ((uint64_t)mac_addr_arr[5] << 0) |
            ((uint64_t)mac_addr_arr[4] << 8) |
            ((uint64_t)mac_addr_arr[3] << 16) |
            ((uint64_t)mac_addr_arr[2] << 24) |
            ((uint64_t)mac_addr_arr[1] << 32) |
            ((uint64_t)mac_addr_arr[0] << 40);
    }

    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "bin_id", mac_addr);
    cJSON_AddNumberToObject(root, "mcu_type", MCU_TYPE);
    cJSON_AddStringToObject(root, "message", message);
    cJSON_AddNumberToObject(root, "fullness", fullness);
    cJSON_AddNumberToObject(root, "overflow", overflow);
    cJSON_AddNumberToObject(root, "weight", weight);
    
    return root;
}

