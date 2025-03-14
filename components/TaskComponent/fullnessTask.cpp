#include "fullnessTask.hpp"
#include "DistanceBuffer.hpp"
#include "FullnessMetric.hpp"
#include "esp_log.h"
#include <driver/gpio.h>
#include "Client.hpp"

// #define BIN_HEIGHT 1000

using namespace Zotbins;

// ESP32-CAM is 12, WROVER is 22 
const gpio_num_t PIN_TRIGGER = GPIO_NUM_12;

// ESP32-CAM is 13, WROVER is 23 
const gpio_num_t PIN_ECHO = GPIO_NUM_13;
const float BIN_HEIGHT = 100;

const gpio_config_t PIN_TRIGGER_CONFIG = {
    .pin_bit_mask = 0x00001000,
    .mode = GPIO_MODE_OUTPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_ENABLE,
    .intr_type = GPIO_INTR_DISABLE};

const gpio_config_t PIN_ECHO_CONFIG = {
    .pin_bit_mask = 0x00002000,
    .mode = GPIO_MODE_INPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE};

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

void FullnessTask::setup() // could refactor into setup later but there are a lot of issues with scope
{
}

float FullnessTask::getFullness(){
    distance = ultrasonic.getDistance();
    return distance; 
}

void FullnessTask::loop()
{
    ESP_ERROR_CHECK(gpio_config(&PIN_TRIGGER_CONFIG));
    ESP_ERROR_CHECK(gpio_config(&PIN_ECHO_CONFIG));

    // TODO: use distance buffer to get averages and discard outliers
    // uint32_t bin_height = BIN_HEIGHT; // TODO: NEED TO OVERLOAD CONSTRUCTOR TO SUPPORT MAX_DISTANCE
    gpio_set_direction(PIN_TRIGGER, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_ECHO, GPIO_MODE_OUTPUT);


    float distance;
    // TODO: this is the bad thing, if not inited it will not allow getDistance
    // find a way to initialize this via constructor
    Fullness::Distance ultrasonic(PIN_TRIGGER, PIN_ECHO);
    
    // ensure comm pins are both low
    gpio_set_level(GPIO_NUM_13, 0);
    gpio_set_level(GPIO_NUM_14, 0);

    while (1)
    {
        // TODO: for now, ultrasonic only sends on GPIO read to HIGH 
        int level = gpio_get_level(GPIO_NUM_13);  
		// ESP_LOGI(name, "level: %d", level);

		// Read input GPIO on HI (meaning trigger)
		if (level == 1){
            ESP_LOGI(name, "Hello from Fullness Task");
            // ulTaskNotifyTake(pdTRUE, (TickType_t)portMAX_DELAY);
            distance = ultrasonic.getDistance();
            ESP_LOGI(name, "Got distance in m: %f", distance);
            
            // TODO: Publish to MQTT broker when done 
            Client::clientPublish("distance", static_cast<void*>(&distance));
            // xTaskToNotify = xTaskGetHandle("weightTask");
            // xTaskNotifyGive(xTaskToNotify);
            gpio_set_level(PIN_TRIGGER, 0);
            gpio_set_level(PIN_ECHO, 0);
            
            // set GPIO back to CAMERA to HIGH (telling it to resume its own task)
            gpio_set_level(GPIO_NUM_14, 1);
            vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay for 1000 milliseconds
            gpio_set_level(GPIO_NUM_14, 0);

            // // TODO: remove if no longer needed
            // xTaskToNotify = xTaskGetHandle("usageTask");        
            // vTaskResume(xTaskToNotify);
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);  
    }
    vTaskDelete(NULL);
}
