#include <Arduino.h>
#include "fullnessTask.hpp"
#include "usageTask.hpp"
#include "weightTask.hpp"
#include "wifiTask.hpp"

#define ZBIN_CORE_VERSION "0.0.1"
#define ZBIN_DISABLE_WIFI

void setup()
{
    log_i("ZotBins Core Version: %s", ZBIN_CORE_VERSION);

    // Create tasks
    Zotbins::FullnessTask fullnessTask;
    Zotbins::UsageTask usageTask;
    Zotbins::WeightTask weightTask;
    #ifndef ZBIN_DISABLE_WIFI
    Zotbins::WiFiTask wifiTask;
    #endif

    // TODO: Create task for MQTT

    // Start all tasks
    fullnessTask.start();
    usageTask.start();
    weightTask.start();
    #ifndef ZBIN_DISABLE_WIFI
    wifiTask.start();
    #endif
}

void loop()
{
    // Do nothing. Things will be done in tasks
}