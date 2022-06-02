#include "fullnessTask.hpp"
#include "FullnessMetric.hpp"
#include "MockDistance.hpp"
#include "UltrasonicDistance.hpp"
#include <Arduino.h>
#include <vector>

using namespace Zotbins;

static const char *name = "fullnessTask";
static const int priority = 1;
static const uint32_t stackSize = 4096;

const int trigPin = 5;
const int echoPin = 18;

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
        log_i("made it 1");
        Fullness::UltrasonicDistance ultrasonicSensor(trigPin, echoPin, 0);
        log_i("made it 2");
        ultrasonicSensor.startDistance();
        log_i("made it 3");
        Fullness::FullnessMetric fullness(1000, ultrasonicSensor);
        log_i("made it 4");
        log_i("Fullness: %f", fullness.getFullness());
    }
}