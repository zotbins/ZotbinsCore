#include "serialize.hpp"
#include <esp_mac.h>

cJSON *serialize(float fullness, float weight, int usage) // Copied from oldzotbinscore
{

    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "bin_id", "newzotbin"); // TODO: change to MAC address after senior design, also consider security implications of attaching messages with a MAC address
    cJSON_AddNumberToObject(root, "fullness", fullness);
    cJSON_AddNumberToObject(root, "weight", weight);
    cJSON_AddNumberToObject(root, "usage", usage);

    return root;
}