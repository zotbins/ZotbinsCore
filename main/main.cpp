#include <stdio.h>
#include <iostream>
#include "Client.hpp"
#include "MockWeight.hpp"
#include "RealWeight.hpp"
#include "fullnessTask.hpp"
#include "weightTask.hpp"
#include "usageTask.hpp"
#include "message.hpp"
#include <driver/gpio.h>

constexpr size_t messageQueueSize = 20;

extern "C" void app_main(void)
{

	// PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[xx], PIN_FUNC_GPIO);
	/*
	Fullness::MockDistance md{std::vector<int32_t>{1, 2}};
	md.getDistance();
	Weight::MockWeight mw{10};
	mw.getWeight();
	Weight::RealWeight rw;
	rw.getWeight();
	*/
    QueueHandle_t messageQueue = xQueueCreate(messageQueueSize, sizeof(Zotbins::Message));
    assert(messageQueue != nullptr);


	Zotbins::FullnessTask fullnessTask(messageQueue);
	fullnessTask.start();
    // Zotbins::WeightTask weightTask(messageQueue);
	// weightTask.start();
	Zotbins::UsageTask usageTask(messageQueue);
	usageTask.start();

	

	// Zotbins::WeightTask weightTask(messageQueue);
	// weightTask.start();
}
