# Introduction

The purpose of this document is to provide a detailed overview of the software architecture behind the new ZotBins Core, focusing on reliability, maintainability, and efficiency.

The intention of this document is to help the embedded system development team understand how the system is structured.

# Background

ZotbinsCore is an IoT (Internet of Things) "smart bin" which currently collects valuable waste metrics on the *University of California, Irvine* campus. A smart bin is a waste bin equipped with intelligent hardware—our smart bin is called the ZotBin.

ZotbinsCore collects waste metrics such as image data and bin usage, interfacing with the **[Zotbins App](https://apps.apple.com/us/app/zotbins/id6743295314)** to help users locate bins and properly dispose their trash. From an organizational standpoint, ZotbinsCore can also be extremely useful for analyzing waste footprint and exposing insightful trends that can help to correct improper practices.

Once the data is collected, it is relayed to our AWS IoT MQTT (Message Queuing Telemetry Transport) Broker which allows us to seamlessly connect an entire network of ZotBins to collect data across an entire campus.

This new iteration uses a single ESP32 microcontroller development board.

# Functional Requirements

Collecting the above metrics requires at least one and sometimes multiple external peripherals. These peripherals are detailed below.

- Usage: a breakbeam sensor counts the instances of a trash item being disposed in the bin, and trigger the sequence of other processes which collect the remaining metrics
- Weight: an array of load cell sensors and a HX711 load cell amplifier collect the weight of the trash item
- Fullness: an ultrasonic sensor measures the amount of empty space in the bin to calculate the ZotBin's percentage fullness

Once each of these metrics are collected, they are assembled into a serialized JSON packet and sent to our database.

# Architecture Overview

ZotbinsCore is an [RTOS](https://www.freertos.org) application emphasizing maintainability, modularity, and reliability. By compartmentalizing each waste metric to its own "[task](https://www.freertos.org/Documentation/02-Kernel/02-Kernel-features/01-Tasks-and-co-routines/03-Task-priorities)", the system is more reliable; halting one task does not halt the others. In addition, the system is more extensible; new features do not interfere with any existing features.

We use [ESP-IDF](https://idf.espressif.com) (Integrated Development Framework) to build ZotBins core into a format that we can flash onto the ESP-32 controllers. ESP-IDF uses [CMake](https://cmake.org), a build system which allows us to manage a large number of files and interacting parts with ease.

ESP-IDF uses [components](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-guides/build-system.html) to organize the individual parts of a total system. ESP-IDF provides a large number of native components, and when we write code to interface with peripherals, we make our own components to keep the codebase organized.