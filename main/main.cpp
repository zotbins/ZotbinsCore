#include "Client.hpp"
#include "cameraTask.hpp"
#include "fullnessTask.hpp"
#include "message.hpp"
#include "servoTask.hpp"
#include "usageTask.hpp"
#include "weightTask.hpp"
#include <iostream>
#include <stdio.h>
#include <driver/gpio.h>

constexpr size_t messageQueueSize = 20;

extern "C" void app_main(void)
{

    Client::clientStart();

    QueueHandle_t messageQueue = xQueueCreate(messageQueueSize, sizeof(Zotbins::Message));
    assert(messageQueue != nullptr);

    // Zotbins::CameraTask cameraTask(messageQueue);
    //  usageTask.start();
    // cameraTask.start();
    Zotbins::WeightTask weightTask(messageQueue);
    weightTask.start();
    Zotbins::FullnessTask fullnessTask(messageQueue);
    fullnessTask.start();
    Zotbins::UsageTask usageTask(messageQueue);
    usageTask.start();

    // Zotbins::ServoTask servoTask(messageQueue);
    // servoTask.start();
}
