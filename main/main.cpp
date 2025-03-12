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
#include "Serialize.hpp"

constexpr size_t messageQueueSize = 20;

extern "C" void app_main(void)
{

    Client::clientStart();

    QueueHandle_t messageQueue = xQueueCreate(messageQueueSize, sizeof(Zotbins::Message));
    assert(messageQueue != nullptr);

	#if MCU_TYPE == CAMERA
		Zotbins::CameraTask cameraTask(messageQueue);
		cameraTask.start();

	#elif MCU_TYPE == SENSOR

		// order matters, weight > fullness > usage
		Zotbins::WeightTask weightTask(messageQueue);
		weightTask.start();

		Zotbins::FullnessTask fullnessTask(messageQueue);
		fullnessTask.start();

		Zotbins::UsageTask usageTask(messageQueue);
		usageTask.start();

	#endif     
}
