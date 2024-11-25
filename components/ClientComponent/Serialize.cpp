#include "Serialize.hpp"

cJSON* Client::serialize(float fullness, int32_t weight) {
    cJSON* root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "fullness", fullness);
    cJSON_AddNumberToObject(root, "weight", weight);
    return root;
}
