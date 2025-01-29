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
    QueueHandle_t messageQueue = xQueueCreate(messageQueueSize, sizeof(Zotbins::Message));
    assert(messageQueue != nullptr);

	Zotbins::WeightTask weightTask(messageQueue);
	weightTask.start();
	Zotbins::FullnessTask fullnessTask(messageQueue);
	fullnessTask.start();
	Zotbins::UsageTask usageTask(messageQueue);
	usageTask.start();
}
