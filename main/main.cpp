#include "Client.hpp"
#include "cameraTask.hpp"
#include "gpsTask.hpp"
#include "fullnessTask.hpp"
#include "message.hpp"
#include "servoTask.hpp"
#include "usageTask.hpp"
#include "weightTask.hpp"
#include <driver/gpio.h>
#include <iostream>
#include <stdio.h>

constexpr size_t messageQueueSize = 20;

extern "C" void app_main(void)
{

    // Client::clientStart();

    QueueHandle_t messageQueue = xQueueCreate(messageQueueSize, sizeof(Zotbins::Message));
    assert(messageQueue != nullptr);

    // Zotbins::CameraTask cameraTask(messageQueue);
    // cameraTask.start();

	ESP_LOGI("main", "Starting fullness task...");
    Zotbins::FullnessTask fullnessTask(messageQueue);
    fullnessTask.start();

	ESP_LOGI("main", "Starting weight task...");
    Zotbins::WeightTask weightTask(messageQueue);
    weightTask.start();

	ESP_LOGI("main", "Starting usage task..."); // I believe usage task needs to be last to let the other tasks set up their task notifications
    Zotbins::UsageTask usageTask(messageQueue);
    usageTask.start();
	
	// Zotbins::CameraTask cameraTask(messageQueue);
	// cameraTask.start();

	// Zotbins::GpsTask gpsTask(messageQueue);
	// gpsTask.start();

	/*
	Zotbins::UsageTask usageTask(messageQueue);
	usageTask.start();

	Zotbins::FullnessTask fullnessTask(messageQueue);
	fullnessTask.start();

  	Zotbins::ServoTask servoTask(messageQueue);
  	servoTask.start();

	Zotbins::WeightTask weightTask(messageQueue);
	weightTask.start();
	*/

    // Zotbins::ServoTask servoTask(messageQueue);
    // servoTask.start();

}
