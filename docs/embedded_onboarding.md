# Introduction
The purpose of this document is to provide a detailed overview of the software architecture behind the new ZotBins Core, focusing on reliability, maintainability, and efficiency.

The intention of this document is to help the embedded system development team understand how the system is structured.

![ZotBins Logo](https://github.com/zotbins/ZotbinsCore/blob/onboarding-docs/docs/images/ZotBins.jpg)

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

# Onboarding Checklist
Before you are given your first assignment as a Zotbins embedded member, we ask that you complete this brief onboarding checklist. This onboarding checklist goes over the basics of an embedded system and goes in more detail about the specific modules we use at ZotBins. If you have any issues going through the equipment, feel free to reach out to any of the embedded leads!

# Section 0: Becoming a Part of ZotBins
ZotBins uses a software called Slack to communicate. We ask that all technical communication between members takes place on the ZotBins Slack. Confirm that you have joined Slack through an invitation from your subteam lead. 

ZotBins also has a Google Drive. Confirm that your subteam lead has given you editing permission. 

# Section I: Embedded Systems

The following stuff is **VERY fundamental** to being able to quickly complete tasks that you'll be assigned as a Zotbinner (I coined that term literally no one on the team calls us that). In the next section, we will go more into depth about the exact implementation of ZotinsCore

> **Important: If you want to follow along, skip to Section II PartI I to set up your developer 
environment!**

The following stuff is **VERY fundamental** to be able to quickly complete tasks that you'll be assigned as a Zotbinner (I coined that term literally no one on the team calls us that). In the next section, we will go more into depth about the exact implementation of ZotinsCore. Section I is more about getting a grasp of the basics.

### Micro-Controller Pinouts
The "pinouts" on a microcontroller refer to the various pins on a microcontroller that expose various functions of the actual microcontroller IC (integrated circuit) or any other peripherals which are commonly included on an ESP-32 microcontroller board like the ones you can purchase directly from Espressif. Common examples of such peripherals are Analog-to-Digital and Digital-to-Analog convertors, SPI or I2C modules (serial protocols), or GPIO (general-purpose in/out) pins.

**Pinout Diagram**

### Configuring GPIO Pins in ESP-IDF
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

Use the Espressif docs _frequently_. It’s like a map of all the building blocks you can use and modify. Every function you'll ever use should have detailed documentation on Espressif's website for data types, parameters, output types, proper error handling, etc.







# Section II: The ZotBins Core
## Part I: Environment Setup

ZotBins Core uses the ESP-IDF framework. Most of us work inside of VS-Code and it is advised that you do the same. Refer to the following [ZotBins tutorial](https://wholesale-grin-d5b.notion.site/ZotBins-Core-ESP32-Developer-Guide-ee6eb8f6678f4cc8927f6f8aa0e22fba) on how to set up your ESP-IDF environment.

If the configuration screen does not pop up after installing the extension, you can type this into the VScode search bar.

__Add Image__

### Cloning the ZotBins Core
All the ZotBins embedded work takes place on the ZotBins Core GitHub, which can be found [here](https://github.com/zotbins/ZotbinsCore/tree/main). Here is a short one minute video on how to clone a repository directly in VScode: [Video](https://www.youtube.com/watch?v=bz1KauFlbQI)

### Building, Flashing, and Monitoring the Code
​The current version of ZotBins uses two different microcontrollers: the ESP32-Cam and the ESP32 Wrover. Flashing is the process of transferring writing a binary program onto non-voltage memory of an embedded system. Effectively its flashing means transferring the program from your computer to the microcontroller.

**Add Image**

The USB-To-TTL plugs into your computer and connects to the microcontroller through the various wires. There are five pins on the module:

**USB-To-TTL Module:**
- 5V - Outputs 5V
- GND - Common Ground
- 3.3V - Outputs 3.3V
- TX - Transmitter Pin (UART Communication)
- RX - Receiver Pin (UART Communication)

​To power and flash the code onto the microcontroller we need all the pins except the 3.3V pin. In order to flash code, we want to match the connections from the USB-To-TTL module to the pins on the ESP (this will vary depending on the microcontroller).

**ESP32-Cam Pinouts**

**Add Image**


**ESP32 Wrover Pinouts**

**Add Image**

We will match the 5V and GND on the USB-To-TTL with the 5V power and GND on microcontrollers. Most systems will have multiple ground pins, and it is okay to use any of them. Then we will connect the UART communication pins. In a UART connection, the TX of device 1 matches with the RX of device 2 and the RX of device 1 matches with the TX of device 2. 

When you are dealing with the USB-To-TTL, you will also need to trigger the reset while flashing on the ESP32. For the ESP32-Cam, this can be done by connecting a GND pin to pin 0 (refer to pinout).

**Flashing Setup for the ESP32-Cam:**

**Add Image**

The ESP32 Wrover can also be flashed directly with a Micro-USB (avoids a lot of the wiring issues):

**Add Image**

Before flashing, refer to the ESP-IDF project configurations at the bottom of the VScode Editor:

**Add Image**

The star icon represents the communication protocol that is used to flash and monitor the code. This should be set to UART. The field next to that is the communication port. The port should be the same as the computer/laptop port that is currently plugged into the USB-To-TTL (this will probably be different from mine). The field next indicates that you are dealing with an ESP32.

C/C++ is a compiler-based program, so before the code is run it needs to be built. Building is the process of turning our computer program into machine code that can be executed by a processor. Every time you make changes to the code, you must build it before executing it. The build may fail for multiple reasons, such as a syntax error. You can build your program by pressing on the wrench button (next to the trashcan and lightning bolt icon). Example of what a successful build might look like in the terminal:

**Add Image**

Once your program has been built and the USB-To-TLL has been set up, we can finally flash the code. Flash the code by pressing on the lightning bolt button. If the flashing process gets stuck in a loop of trying to connect, you may need to press the reset button on the microcontroller.

**Reset Button Image**

Below is an image of what the terminal should say after a successful flash.

**Add Image**

​After a successful flash, the code will start running and can be monitored with the TV symbol (next to lightning bolt). In order for the code to run, we must also take the ESP out of reset mode, so take out the GND to Pin 0 connection. Leave all the other connections untouched. 5V & GND are to power the microcontroller and RX/TX are needed to read/write information to monitor.

A successful monitoring looks differently depending on what the code is doing.

**Add Image**

Congratulations you just ran ZotBins Core!

## Part 2: Components of ZotBins Core
The ZotBins collection is a Smart WasteBin that collects data on the trash that is thrown in. The ZotBins make different measurements through various sensors and peripherals. A brief summary of the different metrics and how we measure them:

- Usage: a breakbeam sensor counts the instances of a trash item being disposed in the bin, and trigger the sequence of other processes which collect the remaining metrics
- Image: a servo-controlled hopper door temporarily traps the trash item for the ESP-32 CAM microcontroller can to take a picture
- Weight: an array of load cell sensors and a HX711 load cell amplifier collect the weight of the trash item
- Fullness: an ultrasonic sensor measures the amount of empty space in the bin to calculate the ZotBin's percentage fullness

Once each of these metrics are collected, they are assembled into a serialized JSON packet and sent to our database.

Below is the current description of the differnet components that are used:

### Breakbeam / Usage Sensor (ADA2168)
We keep track of how often trash is being thrown into the ZotBin. We use a breakbeam sensor (or a usage sensor that we like to call it) to measure the amount. We use the ADA2168 module.

**Add Image**

The breakbeam sensor is made up of two different components: the emitter (clear bulb) and the received (dark bulb). The emitter shoots out an infrared ray (IR) that is directed at the receiver.

If the receiver successfully detects the IR beam, then it will return a logical high (1). If the receiver does not detect the IR beam (because the IR beam's line of sight is broken by an object), then it will return a logical low (0). Refer to the following expertly drawn image.

**Add Image**

**Add Image**

Initially, we used an actual piece of trash to break the beam, but we ran into a lot of issues with people missing the beam entirely. Instead, we attached the breakbeam sensor to the lid. When the lid is closed, it breaks the beam, then when it is opened, it re-establishes the beam. We count this as usage.

**ADA2168 Transmitter Pins:**
- Red Wire - Power (3.3V)
- Black Wire - Common Ground

**ADA2168 Receiver Pins:**
- Red Wire - Power (3.3V)
- Black Wire - Common Ground 
- Yellow Wire - Data Wire (tells user if beam is broken or not)

### Fullness / Ultrasonic Sensor (HC-SR04)
ZotBins keeps track of how full each bin is. We accomplish this through a fullness / ultrasonic sensor. The ultrasonic sensor shoots out a series of ultrasonic pulses. These waves travel and reflect off of the nearest surface. The reflected waves eventually return to the sensor which will measure the time it took.

**Add Image**

Used some mathematics we convert to the time it takes to travel into a distance allowing us to determine how full the bin is.

**Ultrasonic Image**

**HC-SR04 Ultrasonic Pins:**
- VCC - Power (5V)
- Trig(Trigger) - Input pin. If this pin is kept high for 10 us it will initialize the process of sending ultrasonic waves
- Echo -  Output pin. The pin is high for as long as the ultrasonic wave takes to return to the sensor
- GND - Common Ground

### Weight Sensor (Load Cells & HX711 Amplifier):
The weight sensor is used to measure the weight of the trash inside the bin. We pair load cells and a HX711 Amplifier to create a digital scale. Load cells are electromechanical components that will change their resistance depending on how much force is placed on top of them. Below is a picture of the load cell we use:

**Load Cell Image**

The resistance scales linearly with respect to the load that the sensor experiences. Therefore, via a mathematical expression, we relate the weight of objects to the voltage output of the load cells. We use four different load cell units in a wheatstone bridge formation to create an even surface to measure the trash weight.

**Load Cell Image**

SG1-4 are the resistance that are determined by the strain experienced by each load cell. The signal from the load cell configuration may be faint so we attach their outputs to a HX711 Amplifier which boosts the signal so it can easily be deciphered.

**HX711 Amplifier Pins:**

**HX711 Image**

The complete weight sensor looks like the following:

**Complete Image**

### Camera System (ESP32-Cam with the OV2640, Subject to Change):
The Camera System is currently subject to change. Previously, we used the ESP32-Cam, which is a microcontroller which has an onboard camera slot. Most of the time, we slotted in the OV2640 camera module. By using a script on the microcontroller, we could use the camera to take a picture with no external peripheral needed.


### Servo System (Subject to Change)
The ZotBins system includes a servo-controlled lid. 

### GPS Sensor (Air530) 
The ZotBin also includes a GPS sensor that is used to keep track of where it is. The blue component is the controller and the green component is the antenna. For optimal performance, the antenna should have line-of-sight with the sky and be free of any obstructions.

**GPS Sensor Image**

**Air530 GPS Pins:**
- 5V - 5V Power
- GND - Ground
- TX - transmitter pin (UART Communication)
- RX - receiver pin (UART Communication)

### Solar Panels / Charging System
ZotBins is currently developing a way to power our system with solar. The design is done, but still needs more testing. Will follow through with more information later.

## Part 3: Architecture of ZotBins

### Introduction

ZotbinsCore is an [RTOS](https://www.freertos.org) application emphasizing maintainability, modularity, and reliability. By compartmentalizing each waste metric to its own "[task](https://www.freertos.org/Documentation/02-Kernel/02-Kernel-features/01-Tasks-and-co-routines/03-Task-priorities)", the system is more reliable; halting one task does not halt the others. In addition, the system is more extensible; new features do not interfere with any existing features.

We use [ESP-IDF](https://idf.espressif.com) (Integrated Development Framework) to build ZotBins core into a format that we can flash onto the ESP-32 controllers. ESP-IDF uses [CMake](https://cmake.org), a build system which allows us to manage a large number of files and interacting parts with ease.

ESP-IDF uses [components](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-guides/build-system.html) to organize the individual parts of a total system. ESP-IDF provides a large number of native components, and when we write code to interface with peripherals, we make our own components to keep the codebase organized. As of now, having just freshly implemented most of our peripherals, most of the code lies in the TaskComponent, which is intended to keep only RTOS task-related code. In the future, for example, the code related to collecting images would be in a "camera" or "CameraTask" component; the code related to collecting usage data would be in a "usage" or "UsageTask" component, etc. Components such as "hx711" or "ultrasonic" contain external code that others have written that we use to interface with the ZotBin's peripherals.

