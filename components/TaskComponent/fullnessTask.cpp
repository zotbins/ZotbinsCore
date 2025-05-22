#include "fullnessTask.hpp"
#include "DistanceBuffer.hpp"
#include "FullnessMetric.hpp"
#include "esp_log.h"
#include <driver/gpio.h>
#include "Client.hpp"

// #define BIN_HEIGHT 1000

using namespace Zotbins;

// TRIGGER: ESP32-CAM is 12, WROVER is 22 
// ECHO:    ESP32-CAM is 13, WROVER is 23 
#if defined(SENSOR)
    const gpio_num_t PIN_TRIGGER = GPIO_NUM_22;
    const gpio_num_t PIN_ECHO = GPIO_NUM_23;
#elif defined(CAMERA)
    const gpio_num_t PIN_TRIGGER = GPIO_NUM_12;
    const gpio_num_t PIN_ECHO = GPIO_NUM_13;
#endif

const float BIN_HEIGHT = 100; // don't remember what the units are. someone confirm this and rename the constant appropriately

const gpio_config_t PIN_TRIGGER_CONFIG = {
    .pin_bit_mask = (1ULL << PIN_TRIGGER),
    .mode = GPIO_MODE_OUTPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_ENABLE,
    .intr_type = GPIO_INTR_DISABLE
};

const gpio_config_t PIN_ECHO_CONFIG = {
    .pin_bit_mask = (1ULL << PIN_ECHO),
    .mode = GPIO_MODE_INPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE
};

static const char *name = "fullnessTask";
static const int priority = 1;
static const uint32_t stackSize = 4096;

static TaskHandle_t xTaskToNotify = NULL;
static const int core = 1;

FullnessTask::FullnessTask(QueueHandle_t &messageQueue)
    : Task(name, priority, stackSize), mMessageQueue(messageQueue),
      ultrasonic(PIN_TRIGGER, PIN_ECHO) // TODO: Initialize directly, this apparently does not work
{
}

void FullnessTask::start()
{
    xTaskCreatePinnedToCore(taskFunction, mName, mStackSize, this, mPriority, &xTaskToNotify, core);
}

void FullnessTask::taskFunction(void *task)
{
    FullnessTask *fullnessTask = static_cast<FullnessTask *>(task);
    fullnessTask->setup();
    fullnessTask->loop();
}

void FullnessTask::setup()
{
    ESP_ERROR_CHECK(gpio_config(&PIN_TRIGGER_CONFIG)); // NECESSARY FOR SOME PINS!!
    ESP_ERROR_CHECK(gpio_config(&PIN_ECHO_CONFIG));
}

float FullnessTask::getFullness(){
    distance = ultrasonic.getDistance();
    return distance; 
}

void FullnessTask::loop()
{

    ESP_LOGI(name, "Hello from Fullness Task");

    // TODO: use distance buffer to get averages and discard outliers
    // TODO: use BIN_HEIGHT to calculate a percentage fullness

    float distance;
    Fullness::Distance ultrasonic(PIN_TRIGGER, PIN_ECHO);
    
    // ensure trigger pin is low
    gpio_set_level(PIN_TRIGGER, 0);

    // float threshold = 0.35;
    // bool SENT = false;
    
    while (1)
    {

        ulTaskNotifyTake(pdTRUE, (TickType_t)portMAX_DELAY);

        // TODO: for now, ultrasonic only sends on GPIO read to HIGH
        distance = ultrasonic.getDistance();
        ESP_LOGI(name, "Got distance in m: %f", distance);
        Client::clientPublish("distance", static_cast<void*>(&distance));

        // if(distance > threshold && SENT == false){
        //     Client::clientPublish("distance", static_cast<void*>(&distance));
        //     ESP_LOGI(name, "Sent Distance %f", distance);
        //     SENT = true;
        // }
        // else if (distance < threshold && SENT == true){
        //     SENT = false; 
        // }

        gpio_set_level(PIN_TRIGGER, 0);

        /* task notifications */

        // // if weightTask is enabled
        xTaskToNotify = xTaskGetHandle("weightTask"); 
        xTaskNotifyGive(xTaskToNotify); // once fullness is collected notify weight
        ESP_LOGI(name, "Notified Weight Task");
        // vTaskSuspend(NULL);

        // if weightTask is disabled
        // xTaskToNotify = xTaskGetHandle("usageTask");      
        // vTaskResume(xTaskToNotify);
        // ESP_LOGI(name, "Notified Usage Task");


        
    }
    vTaskDelete(NULL);
}
