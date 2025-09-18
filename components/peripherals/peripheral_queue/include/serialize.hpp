#ifndef SERIALIZE_HPP
#define SERIALIZE_HPP

#include <cJSON.h>
#include <stdint.h>

cJSON *serialize(float fullness, float weight, int usage);

#endif // SERIALIZE_HPP
