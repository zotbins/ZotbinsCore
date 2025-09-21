#ifndef SERIALIZE_HPP
#define SERIALIZE_HPP

#include <cJSON.h>
#include <stdint.h>

/**
 * @brief Serializes input data as a JSON string.
 * 
 * @param fullness 
 * @param weight 
 * @param usage 
 * @return char* 
 */
char *serialize(float fullness, float weight, int usage);

#endif // SERIALIZE_HPP
