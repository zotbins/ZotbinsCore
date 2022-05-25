#include "fullnessTask.hpp"
#include "FullnessMetric.hpp"
#include "MockDistance.hpp"
#include <Arduino.h>
#include <vector>

using namespace Zotbins;

static const char *name = "fullnessTask";
static const int priority = 1;
static const uint32_t stackSize = 4096;

const int trigPin = 5;
const int echoPin = 18;
int amount = 0;
std::vector<int32_t> distances;

FullnessTask::FullnessTask(QueueHandle_t &messageQueue)
    : Task(name, priority, stackSize), mMessageQueue(messageQueue)
{
}

void FullnessTask::start()
{
    xTaskCreate(taskFunction, mName, mStackSize, this, mPriority, nullptr);
}

void FullnessTask::taskFunction(void *task)
{
    FullnessTask *fullnessTask = static_cast<FullnessTask *>(task);
    fullnessTask->setup();
    fullnessTask->loop();
}

void FullnessTask::setup()
{
    Serial.begin(115200);
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
}

void FullnessTask::loop()
{
    while (1)
    {
        if (amount < 8)
        {
            digitalWrite(trigPin, LOW);
            vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay for 1000 milliseconds
            digitalWrite(trigPin, HIGH);
            vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay for 1000 milliseconds
            digitalWrite(trigPin, LOW);

            // read echo pin
            long duration = pulseIn(echoPin, HIGH);
            int32_t intDuration = duration * .034 / 2;
            log_i("Ultrasonic sensor reading: %d", intDuration);

            // adding to vector
            distances.push_back(duration);
            amount++;
        }
        else
        {
            Fullness::MockDistance distanceSensor(distances);
            Fullness::FullnessMetric fullness(1000, distanceSensor);

            log_i("Fullness: %f", fullness.getFullness());
            amount = 0;
        }

        // log_i("Hello from Fullness Task");

        // distance sensor
        // Fullness::IDistance distanceSensor;

        // log_i();
    }
}