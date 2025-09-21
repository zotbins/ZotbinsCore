/**
 * @file serialize.cpp
 * @author Alex Ikeda (ikedaas@uci.edu)
 * @brief Serializes data using cJSON library.
 * @version 0.1
 * @date 2025-09-20
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "serialize.hpp"

char *serialize(float fullness, float weight, int usage) // Copied from oldzotbinscore
{

    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "bin_id", "newzotbin"); // Attach bin ID
    cJSON_AddNumberToObject(root, "fullness", fullness); // Attach fullness (in centimeters) TODO: change to percentage
    cJSON_AddNumberToObject(root, "weight", weight); // Attach weight (in grams) TODO: calibrate
    cJSON_AddNumberToObject(root, "usage", usage); // Attach usage (number of trash items)

    char *payload = cJSON_PrintUnformatted(root); // Convert JSON object to string
    cJSON_Delete(root); // Free JSON object

    return payload;
}