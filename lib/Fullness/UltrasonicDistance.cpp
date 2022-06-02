#include "UltrasonicDistance.hpp"
#include <Arduino.h>

using namespace Fullness;

UltrasonicDistance::UltrasonicDistance(int32_t trig, int32_t echo, size_t distanceIdx)
    : trigPin{trig}, echoPin{echo}, mDistanceBufferIdx{distanceIdx}
{
}

int32_t UltrasonicDistance::startDistance()
{
    for (int i = 0; i < 8; i++)
    {
        digitalWrite(trigPin, LOW);
        vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay for 1000 milliseconds
        digitalWrite(trigPin, HIGH);
        vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay for 1000 milliseconds
        digitalWrite(trigPin, LOW);

        long duration = pulseIn(echoPin, HIGH);
        int32_t intDuration = duration * .034 / 2;

        distances[i] = intDuration;
    }
}

int32_t UltrasonicDistance::getDistance()
{
    int32_t distance = distances[mDistanceBufferIdx];
    mDistanceBufferIdx = (mDistanceBufferIdx + 1) % distances.size();
    return distance;
}