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

    Client::clientStart();

    QueueHandle_t messageQueue = xQueueCreate(messageQueueSize, sizeof(Zotbins::Message));
    assert(messageQueue != nullptr);

    // Zotbins::CameraTask cameraTask(messageQueue);
    // cameraTask.start();

    Zotbins::FullnessTask fullnessTask(messageQueue);
    fullnessTask.start();

    // Zotbins::WeightTask weightTask(messageQueue);
    // weightTask.start();

    Zotbins::UsageTask usageTask(messageQueue);
    usageTask.start();
    // Zotbins::WeightTask weightTask(messageQueue);
	// weightTask.start();
	// Zotbins::UsageTask usageTask(messageQueue);
	// usageTask.start();
	
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
