#include <Arduino.h>

#include "fullnessTask.hpp"
#include "message.hpp"
#include "usageTask.hpp"
#include "weightTask.hpp"
#include "wifiTask.hpp"

#define ZBIN_CORE_VERSION "0.0.1"
#define ZBIN_DISABLE_WIFI

constexpr size_t messageQueueSize = 20;

void setup()
{
    log_i("ZotBins Core Version: %s", ZBIN_CORE_VERSION);

    // Create message queue for inter-task communication
    QueueHandle_t messageQueue = xQueueCreate(messageQueueSize, sizeof(Zotbins::Message));
    assert(messageQueue != nullptr);

    // Create tasks
    Zotbins::FullnessTask fullnessTask(messageQueue);
    Zotbins::UsageTask usageTask(messageQueue);
    Zotbins::WeightTask weightTask(messageQueue);
#ifndef ZBIN_DISABLE_WIFI
    Zotbins::WiFiTask wifiTask(messageQueue);
#endif

    // Start all tasks
    //fullnessTask.start();
    //usageTask.start();
    weightTask.start();
#ifndef ZBIN_DISABLE_WIFI
    wifiTask.start();
#endif
}

void loop()
{
    // Do nothing. Things will be done in tasks
}