#include "Client.hpp"
#include "cameraTask.hpp"
#include "gpsTask.hpp"
#include "fullnessTask.hpp"
#include "message.hpp"
#include "servoTask.hpp"
#include "usageTask.hpp"
#include "communicationTask.hpp"
#include "weightTask.hpp"
#include <driver/gpio.h>
#include <iostream>
#include <stdio.h>
#include "Serialize.hpp"

constexpr size_t messageQueueSize = 20;

static const char *name = "main";

extern "C" void app_main(void)
{

    Client::clientStart();

    QueueHandle_t messageQueue = xQueueCreate(messageQueueSize, sizeof(Zotbins::Message));
    assert(messageQueue != nullptr);	

	#if MCU_TYPE == CAMERA

		// WARNING: ENABLE PSRAM BEFORE USE
		// order matters, usage last

		ESP_LOGW(name, "MCU_TYPE is set as camera. Running camera config.");

		// Zotbins::ServoTask servoTask(messageQueue);
		// servoTask.start();		
		
		// Zotbins::CommunicationTask communicationTask(messageQueue);
		// communicationTask.start();

		// Zotbins::CameraTask cameraTask(messageQueue);
		// cameraTask.start();

		// Zotbins::UsageTask usageTask( messageQueue);
		// usageTask.start();

	#elif MCU_TYPE == SENSOR

		ESP_LOGW(name, "MCU_TYPE is set as sensor. Running sensor config.");

		// WARNING: DISABLE PSRAM BEFORE USE
		// order matters, weight > fullness > usage

		Zotbins::WeightTask weightTask(messageQueue);
		weightTask.start();
 
		Zotbins::FullnessTask fullnessTask(messageQueue);
		fullnessTask.start();

		Zotbins::UsageTask usageTask(messageQueue);
		usageTask.start();
    
	#endif     
}
