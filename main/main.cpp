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

	// start mqtt task
    Client::clientStart();

    QueueHandle_t messageQueue = xQueueCreate(messageQueueSize, sizeof(Zotbins::Message));
    assert(messageQueue != nullptr);	

	// // not using communication task in favor of breakbeam interrupting both esps simultaneously, at a hardware level
	// Zotbins::CommunicationTask communicationTask(messageQueue);
	// communicationTask.start();

	/* esp device specific tasks */
	#if defined(CAMERA)

		ESP_LOGW(name, "MCU_TYPE is set as camera. Running camera config.");

		// WARNING: ENABLE PSRAM BEFORE USE
		// order matters, usage last

		Zotbins::CameraTask cameraTask(messageQueue);
		cameraTask.start();

	#elif defined(SENSOR)

		ESP_LOGW(name, "MCU_TYPE is set as sensor. Running sensor config.");

		// WARNING: DISABLE PSRAM BEFORE USE
		// order matters, weight > fullness > usage

		// Zotbins::ServoTask servoTask(messageQueue);
		// servoTask.start();

		// not using weight for sustainable food fair
		// Zotbins::WeightTask weightTask(messageQueue);
		// weightTask.start();
 
		Zotbins::FullnessTask fullnessTask(messageQueue);
		fullnessTask.start();
    
	#endif     
	/* end of esp device specific tasks */


	ESP_LOGI(name, "Starting usage task...");
	Zotbins::UsageTask usageTask(messageQueue);
	usageTask.start();

	
}
