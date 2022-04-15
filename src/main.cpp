#include <Arduino.h>
#include <FreeRTOS.h>

#define ZBIN_CORE_VERSION "0.0.1"

// These tasks will be removed in a later version
// This is just show how tasks work in FreeRTOS
void dummy_task1(void *params) {
    while (1) {
        log_i("Hello from DummyTask1");

        vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay for 1000 ms
    }
}

void dummy_task2(void *params) {
    while (1) {
        log_i("Hello from DummyTask2");
        vTaskDelay(5000 / portTICK_PERIOD_MS); // Delay for 5000 ms
    }
}

void setup() {
    log_i("ZotBins Core Version: %s", ZBIN_CORE_VERSION);

    // Create tasks to run
    xTaskCreate(dummy_task1, "DummyTask1", 1024, nullptr, 1, nullptr);
    xTaskCreate(dummy_task2, "DummyTask2", 1024, nullptr, 1, nullptr);
}

void loop() {
    // Do nothing. Things will be done in tasks
}