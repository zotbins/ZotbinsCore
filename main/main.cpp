#include <stdio.h>
#include <iostream>
#include "Client.hpp"
#include "MockDistance.hpp"
#include "MockWeight.hpp"
#include "RealWeight.hpp"
<<<<<<< Updated upstream
=======
#include "fullnessTask.hpp"
#include "servoTask.hpp"
>>>>>>> Stashed changes
#include "weightTask.hpp"
#include "usageTask.hpp"
#include "message.hpp"

constexpr size_t messageQueueSize = 20;

extern "C" void app_main(void)
{
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

    // Zotbins::WeightTask weightTask(messageQueue);
	// weightTask.start();
	Zotbins::UsageTask usageTask(messageQueue);
	usageTask.start();
<<<<<<< Updated upstream
=======

	Zotbins::FullnessTask fullnessTask(messageQueue);
	fullnessTask.start();

    Zotbins::ServoTask servoTask(messageQueue);
   servoTask.start();

	Zotbins::WeightTask weightTask(messageQueue);
	weightTask.start();
>>>>>>> Stashed changes
}
