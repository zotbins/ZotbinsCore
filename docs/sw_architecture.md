# Introduction

The purpose of this document is to provide a detailed overview of the software architecture behind the new ZotBins Core, focusing on reliability, maintainability, and efficiency.

The intention of this document is to help the embedded system development team understand how the system is structured.

# Background

ZotbinsCore is an IoT (Internet of Things) "smart bin" which currently collects valuable waste metrics on the *University of California, Irvine* campus. A smart bin is a waste bin equipped with intelligent hardware—our smart bin is called the ZotBin.

ZotbinsCore collects waste metrics such as image data and bin usage, interfacing with the **[Zotbins App](https://apps.apple.com/us/app/zotbins/id6743295314)** to help users locate bins and properly dispose their trash. From an organizational standpoint, ZotbinsCore can also be extremely useful for analyzing waste footprint and exposing insightful trends that can help to correct improper practices.

Currently ZotbinsCore collects the following metrics:
- Pictures of trash disposed in the ZotBin 
	- These images are categorized by Zotbins' Waste Recognition team
- Number of items disposed in the ZotBin
- Weight of items disposed in the ZotBin
- Fullness of the ZotBin

Once the data is collected, it is relayed to our AWS IoT MQTT (Message Queuing Telemetry Transport) Broker which allows us to seamlessly connect an entire network of ZotBins to collect data across an entire campus.

The current iteration uses a dual ESP-32 system to support the large number of sensors and prioritize speed. We eventually plan to reduce the cost and footprint of the ZotBin.

# Functional Requirements

Collecting the above metrics requires at least one and sometimes multiple external peripherals. These peripherals are detailed below.

- Usage: a breakbeam sensor counts the instances of a trash item being disposed in the bin, and trigger the sequence of other processes which collect the remaining metrics
- Image: a servo-controlled hopper door temporarily traps the trash item for the ESP-32 CAM microcontroller can to take a picture
- Weight: an array of load cell sensors and a HX711 load cell amplifier collect the weight of the trash item
- Fullness: an ultrasonic sensor measures the amount of empty space in the bin to calculate the ZotBin's percentage fullness

Once each of these metrics are collected, they are assembled into a serialized JSON packet and sent to our database.

# Architecture Overview

ZotbinsCore is an [RTOS](https://www.freertos.org) application emphasizing maintainability, modularity, and reliability. By compartmentalizing each waste metric to its own "[task](https://www.freertos.org/Documentation/02-Kernel/02-Kernel-features/01-Tasks-and-co-routines/03-Task-priorities)", the system is more reliable; halting one task does not halt the others. In addition, the system is more extensible; new features do not interfere with any existing features.

We use [ESP-IDF](https://idf.espressif.com) (Integrated Development Framework) to build ZotBins core into a format that we can flash onto the ESP-32 controllers. ESP-IDF uses [CMake](https://cmake.org), a build system which allows us to manage a large number of files and interacting parts with ease.

ESP-IDF uses [components](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-guides/build-system.html) to organize the individual parts of a total system. ESP-IDF provides a large number of native components, and when we write code to interface with peripherals, we make our own components to keep the codebase organized. As of now, having just freshly implemented most of our peripherals, most of the code lies in the TaskComponent, which is intended to keep only RTOS task-related code. In the future, for example, the code related to collecting images would be in a "camera" or "CameraTask" component; the code related to collecting usage data would be in a "usage" or "UsageTask" component, etc. Components such as "hx711" or "ultrasonic" contain external code that others have written that we use to interface with the ZotBin's peripherals.

# Basics (Onboarding)

##### Microcontroller Pinouts

The "pinouts" on a microcontroller refer to the various pins on a microcontroller that expose various functions of the actual microcontroller IC (integrated circuit) or any other peripherals which are commonly included on an ESP-32 microcontroller board like the ones you can purchase directly from Espressif. Common examples of such peripherals are Analog-to-Digital and Digital-to-Analog convertors, SPI or I2C modules (serial protocols), or GPIO (general-purpose in/out) pins.

##### Configuring GPIO pins in ESP-IDF

ESP-IDF has a custom data type called `gpio_num_t` which is the data type of all the pins you can work with on an ESP-32. Generally, most functions which work with `gpio_num_t` will also work with standard integer types but it is safest to use the `gpio_num_t` type. An example of assigning a pin number to a variable:

`const gpio_num_t PIN_NAME = GPIO_NUM_12;`

This initializes a constant variable (we nearly always want the variable name to refer to the same pin for its entire lifetime, otherwise really bad things could happen) called `PIN_NAME`. `PIN_NAME` is meant to describe the purpose of the pin, in this case `GPIO_NUM_12` or the 12th pin on the ESP-32. For example, if this pin was being used to detect if a breakbeam was broken, you might want to call it something like `PIN_BREAKBEAM`.

Microcontroller board pins generally can be configured to do multiple things. Some pins function as both the input/output of some kind of serial protocol as well as a regular digital GPIO pin. Generally, most pins function as GPIO pins in addition to their other functions. The usual exceptions are the VCC/GND or power pins which supply power to the microcontroller, or any power outputs such as a 3.3V regulator output. **Always** check the microcontroller pinout/datasheet to check whether a pin can be used for a specific purpose. 

While the pins can have multiple functions, you can only use the pin for one of those functions at a time. Therefore, before you use a pin, you need to *configure* it.

The most common pin function is the digital GPIO pin. This pin either outputs a high/low signal (5V or 0V) or reads a signal and interprets it as either high/low (again, 5V or 0V). Here is an example from ZotBins core where we configure a GPIO pin as digital GPIO.

```c++

const gpio_num_t PIN_BREAKBEAM = GPIO_NUM_18;

const gpio_config_t PIN_BREAKBEAM_CONFIG = {
	.pin_bit_mask = (1ULL << PIN_BREAKBEAM),
	.mode = GPIO_MODE_INPUT,
	.pull_up_en = GPIO_PULLUP_ENABLE,
	.pull_down_en = GPIO_PULLDOWN_DISABLE,
	.intr_type = GPIO_INTR_NEGEDGE
};

ESP_ERROR_CHECK(gpio_config(&PIN_BREAKBEAM_CONFIG));

```

`PIN_BREAKBEAM_CONFIG` is the struct we use to tell the microcontroller how we want to configure the pin. It contains the pin's bitmask or the pin number, the mode (input or output) whether we want to add an internal [pull-up or pull-down resistor](https://eepower.com/resistor-guide/resistor-applications/pull-up-resistor-pull-down-resistor/), and whether we want to configure the pin as an interrupt pin (more on that later).


