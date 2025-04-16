#include "Client.hpp"
#include "cameraTask.hpp"
// #include "gpsTask.hpp"
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

    //Client::clientStart();

    QueueHandle_t messageQueue = xQueueCreate(messageQueueSize, sizeof(Zotbins::Message));
    assert(messageQueue != nullptr);	

	#if MCU_TYPE == CAMERA

		// // GPIO13 as OUTPUT (sends signal)
		// // GPIO14 as INPUT (receives signal)
		// gpio_config_t io_conf = {};

		// // Configure GPIO13 as OUTPUT
		// io_conf.pin_bit_mask = (1ULL << GPIO_NUM_13);
		// io_conf.mode = GPIO_MODE_OUTPUT;
		// io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;  
		// io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
		// gpio_config(&io_conf);

		// // Configure GPIO14 as INPUT
		// io_conf.pin_bit_mask = (1ULL << GPIO_NUM_14);
		// io_conf.mode = GPIO_MODE_INPUT;
		// io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;  // Prevent floating
		// gpio_config(&io_conf);

		// Zotbins::ServoTask servoTask(messageQueue);
		// servoTask.start();

		// Zotbins::CameraTask cameraTask(messageQueue);
		// cameraTask.start();

		// Zotbins::UsageTask usageTask( messageQueue);
		// usageTask.start();

	#elif MCU_TYPE == SENSOR
		// GPIO13 as INPUT (receives signal)
		// GPIO14 as OUTPUT (sends signal)
		gpio_config_t io_conf = {};

		// Configure GPIO13 as INPUT
		io_conf.pin_bit_mask = (1ULL << GPIO_NUM_13);
		io_conf.mode = GPIO_MODE_INPUT;
		io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;  // Prevent floating
		gpio_config(&io_conf);

		// Configure GPIO14 as OUTPUT
		io_conf.pin_bit_mask = (1ULL << GPIO_NUM_14);
		io_conf.mode = GPIO_MODE_OUTPUT;
		io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
		io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
		gpio_config(&io_conf);

		// WARNING: DISABLE PSRAM BEFORE USE
		// order matters, weight > fullness > usage
		Zotbins::WeightTask weightTask(messageQueue);
		weightTask.start();

		// Zotbins::FullnessTask fullnessTask(messageQueue);
		// fullnessTask.start();

		// Zotbins::UsageTask usageTask(messageQueue);
		// usageTask.start();
    
	#endif     
}
