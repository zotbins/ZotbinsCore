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

The following stuff is **VERY fundamental** to being able to quickly complete tasks that you'll be assigned as a Zotbinner (I coined that term literally no one on the team calls us that). 

##### Flashing the ESP-32

Most of us at Zotbins are using VSCode with the ESP-IDF extension installed to flash our ESP-32. The extension allows you to install the ESP-IDF framework directly inside of VSCode and makes it super easy to build without ever having to memorize or search any commands to use in the terminal.

> **Important**: Make sure your ESP32 is connected to your computer via USB or one of the USB-TTL (UART) adapters **before** you run the flash command.

Before flashing, your code has to be compiled into a binary that the microcontroller can understand. This is done using `idf.py build` or by clicking the little build button located at the bottom toolbar (it looks like a wrench, and if you hover over it it'll say build).

![/images/Pasted%20image%2020250602230342.png]

After, you need to select the port. This basically tells the extension where on your computer it should try sending the firmware when flashing. Most of the time it's a USB port. (It should look something like the `/dev/tty.usbserial-120` in the screenshot for MacOS. For Linux and Windows, look for something that says USB or COM.)

After this is done, make sure you have the correct serial protocol selected (our adapters use UART) and then click the lightning icon to flash. To see all the of the info messages that the ESP-32 is printing, click on the computer monitor. To do all of this in one step (build, flash, then monitor) click the fire icon.

If you don't want to use the VSCode extension, you can still use the frontend commands found [here](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-guides/tools/idf-py.html).

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

const gpio_num_t PIN_BREAKBEAM = GPIO_NUM_14;

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

Again, **make sure to double check the datasheet for what features can and can't be used** on a pin. For example, some pins have internal pull-up/down resistors; some pins don't. Some pins can be used for SDA/SDL (I2C serial lines) while others can't. And so on. One of the microcontrollers we're working with right now is the ESP32 WROVER-E, for which I have linked the datasheet [here](https://www.espressif.com/sites/default/files/documentation/esp32-wrover-e_esp32-wrover-ie_datasheet_en.pdf).

##### Reading datasheets

This is the kind of thing you're going to learn by doing, but I'll have an example here. Let's find whether GPIO 14 has an internal pull-up resistor.

First I'll go to the ESP32 WROVER-E datasheet. The table of contents is the first place you want to look, and since we're interested in the GPIO pins I'll look for anything that talks about *pins* or *pin functions*. 

![why does github require alt text](images/Pasted%20image%2020250602224718.png)

*Pin Description* and *Peripheral Overview* are good places to start, since the first has the word *pin* in it and the second should be talking about how the GPIO pins can be configured to interface with various peripherals. It turns out you'll find this section in *Peripheral Overview*:

![/images/Pasted%20image%2020250602224848.png]

This tells us to go to the ESP32 Series datasheet, which makes sense since the pull-up resistors are a part of the actual ESP32 integrated circuit chip rather than the development board they come packaged on!

(One useful note to make is the difference between a microcontroller and a development board. "Microcontroller" refers to the actual integrated circuit (IC) which come packaged as these tiny black boxes with a bunch of pins. Most people's first introduction to microcontrollers is through, for example, an Arduino, which is actually just a microcontroller packaged with other components and pin headers, called a *development board*. This development board comes prepackaged with lots of useful features that make prototyping a lot easier, such as decoupling capacitors placed close to the microcontroller IC, pin headers to plug the microcontroller into a breadboard, etc. But again, microcontroller refers to the IC, not the dev board.)

Following those directions, you'll find this table:

![/images/Pasted%20image%2020250602225453.png]

Which tells us GPIO 14 is one of the functions of pin 17, and that pin 17 has the *wpu* function (it has an internal weak pull-up resistor).

##### Reading reference guides and technical documentation

I can't go over every single feature of ESP-IDF, so you'll eventually have to do some reading through the raw documentation. It's not that scary. Let's do a walkthrough.

The [API reference](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/index.html) on Espressif's website has basically everything you need to know. We're going to try to get MQTT working on an ESP-IDF microcontroller, 

This section is a practical walk-through that shows you **how to use the ESP-IDF API reference** to understand and modify real projects. We'll use the `mqtt/ssl` example from Espressif's GitHub and API documentation as our guide.

###### Step 1: Choosing an MQTT Example

If you go to the [ESP-IDF Example Index](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/index.html), you'll find many example projects under **Application Examples**, including a group for `protocols/mqtt`. After navigating there, at first glance, you might see a lot of options:

![/images/Pasted%20image%2020250602232041.png]

How do you know which one to pick? It depends on how you want to connect to your MQTT broker. Search or ask your favorite LLM what these are:

|Example|Description|Port|
|---|---|---|
|`mqtt/tcp`|Basic insecure MQTT over TCP|1883|
|`mqtt/ssl`|Secure MQTT over TLS|8883|
|`mqtt/ssl_mutual_auth`|MQTT with **client and server certificates**|8883|
|`mqtt/ssl_psk`|Secure MQTT using **pre-shared keys** for auth|8883|
|`mqtt/ws`|MQTT over WebSocket (typically for browsers)|80|
|`mqtt/wss`|MQTT over Secure WebSocket|443|
|`mqtt5`|For MQTT version 5.0 (newer spec, not always needed)|1883 or 8883|

ZotBins communicates securely with an **AWS IoT MQTT broker** (you'll probably figure that out by going to general meetings), so we need secure communication (TLS/SSL). We don't really need any of the extra stuff, so you don't even need to know what any of the other protocols or features are and can just pick SSL.

###### Step 2: Download the Example Code

You can clone the ESP-IDF examples (or just this one) directly from Espressif’s GitHub:

`git clone https://github.com/espressif/esp-idf`
`cd esp-idf/examples/protocols/mqtt/ssl`

> Make sure you’re using a version of ESP-IDF compatible with this example—e.g., `v5.4.1`.

###### Step 3: Understand the Code Using the API Reference

Open the [ESP-IDF API Reference](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/index.html).

In the MQTT example, you’ll likely see code like:

```c++
esp_mqtt_client_config_t mqtt_cfg = {
    .uri = CONFIG_BROKER_URI,
};
esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
esp_mqtt_client_start(client);

```
And you'll notice that `CONFIG_BROKER_URI` isn't actually defined anywhere. **One other important** feature of the ESP-IDF build system is that you can configure a REALLY large number of properties in the file `sdkconfig`. This is best done by running the command `idf.py menuconfig` or clicking the cog at the bottom toolbar in VSCode. 

These properties can enable and disable microcontroller features, etc., but what we're most interested in is the Wi-Fi and broker URI.

When you see something like `CONFIG_BROKER_URI`, that's ESP-IDF's naming convention for things that are defined in the `sdkconfig`. So you can configure `CONFIG_BROKER_URI` as well as the Wi-FI SSID and password to get the example up and running. In the case of the example, `CONFIG_BROKER_URI` should already be set to Espressif's example broker, so you just need the Wi-Fi stuff.

Once all of this is done, build, flash and monitor. You should see your ESP-32 connecting successfully in the monitor window.

###### Configuring the example to do other things

Search for: **"esp_mqtt_client_init"** in the API Reference or just Google *esp_mqtt_client_init site:docs.espressif.com* because the official docs are going to be way more accurate than Arduino forums or ChatGPT. Plus it's good practice.

You’ll find the [ESP-MQTT Client API](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/protocols/mqtt.html) page where you can learn:

- What `esp_mqtt_client_config_t` fields you can customize (like TLS certificates, usernames, passwords, etc.)
- How to subscribe to topics (`esp_mqtt_client_subscribe`)
- How to publish messages (`esp_mqtt_client_publish`)
- How callbacks work via `esp_mqtt_client_register_event`

Use the Espressif docs _frequently_. It’s like a map of all the building blocks you can use and modify. Every function you'll ever use should have detailed documentation on Espressif's website for data types, parameters, output types, proper error handling, etc.