#include <Arduino.h>
#include "fullnessTask.hpp"
#include "usageTask.hpp"
#include "weightTask.hpp"

#define ZBIN_CORE_VERSION "0.0.1"

void setup()
{
    log_i("ZotBins Core Version: %s", ZBIN_CORE_VERSION);

    // Create tasks
    Zotbins::FullnessTask fullnessTask;
    Zotbins::UsageTask usageTask;
    Zotbins::WeightTask weightTask;
    // TODO: Create task for MQTT

    // Start all tasks
    fullnessTask.start();
    usageTask.start();
    weightTask.start();
}

void loop()
{
    // Do nothing. Things will be done in tasks
}