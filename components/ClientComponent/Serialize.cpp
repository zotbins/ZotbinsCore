#include "Serialize.hpp"
#include <esp_mac.h>

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
    uint64_t mac_addr = getMAC();

    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "bin_id", mac_addr);
    cJSON_AddNumberToObject(root, "mcu_type", MCU_TYPE);
    cJSON_AddStringToObject(root, "message", message);
    cJSON_AddNumberToObject(root, "fullness", fullness);
    cJSON_AddNumberToObject(root, "overflow", overflow);
    cJSON_AddNumberToObject(root, "weight", weight);

    return root;
}

cJSON *Client::serializeImage(char* message, char* img_str, size_t buffer_length)
{
    uint64_t mac_addr = getMAC();

    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "bin_id", mac_addr);
    cJSON_AddNumberToObject(root, "mcu_type", MCU_TYPE);
    cJSON_AddStringToObject(root, "message", message);
    cJSON_AddStringToObject(root, "img_str", img_str);
    cJSON_AddNumberToObject(root, "buffer_length", buffer_length);
    
    return root;
}


