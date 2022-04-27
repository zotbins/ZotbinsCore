#include "wifiTask.hpp"

#include <Arduino.h>
#include <WiFi.h>

using namespace Zotbins;

// TODO: Move config to separate header file
namespace WiFiConfig
{
    const char *ssid = "";
    const char *pwd = "";
}

static const char *name = "WiFiTask";
static const int priority = 1;
static const uint32_t stackSize = 4096;

WiFiTask::WiFiTask()
    : Task(name, priority, stackSize)
{
}

void WiFiTask::start()
{
    xTaskCreate(taskFunction, mName, mStackSize, this, mPriority, nullptr);
}

void WiFiTask::taskFunction(void *task)
{
    WiFiTask *wifiTask = static_cast<WiFiTask *>(task);
    wifiTask->setup();
    wifiTask->loop();
}

void WiFiTask::setup()
{
    if (setupWifi())
    {
        log_i("WiFi Connected to %s", WiFiConfig::ssid);
    }
    else
    {
        while (1)
        {
            log_e("Cannot connect to WiFi");
            vTaskDelay(10000 / portTICK_PERIOD_MS);
        }
    }
}

void WiFiTask::loop()
{
    while (1)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay for 1000 milliseconds
        log_i("Hello from WiFi Task");
    }
}

bool WiFiTask::setupWifi()
{
    const uint32_t MAX_WIFI_CONNECT_COUNT = 100;
    uint32_t wifiConnectCount = 0;

    WiFi.mode(WIFI_STA);
    WiFi.begin(WiFiConfig::ssid, WiFiConfig::pwd);

    while (!WiFi.isConnected())
    {
        if (wifiConnectCount < MAX_WIFI_CONNECT_COUNT)
        {
            log_d("Attempting to connect to WiFi...");
            ++wifiConnectCount;
        }
        else
        {
            return false;
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay for 1 second
    }
    return true;
}