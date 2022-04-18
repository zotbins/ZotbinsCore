# ZotBins Core Software Architecture

## Introduction
The purpose of this document is to provide a detailed overview of the software architecture behind the new ZotBins Core, focusing on reliability, maintainability, efficiency, and testability. 

The intention of this document is to help the embedded system development team understand how the system is structured.

## Background
ZotBins Core is a IoT Smart Bin to keep track of waste data metrics like bin fullness, weight, usage rates, etc. This is intended to help quantify waste data and promote solutions to reduce the amount of waste people throw away.

The previous iteration was a Raspberry Pi-based system. This system is intended to a replacement for the Raspberry Pi system, while being more reliable, maintainable, efficient, and testable.

## Functional Requirements
Many of the features revolve around the various sensors that the ZotBins Core utilize.

- Breakbeam Sensor: Measure the amount of time a bin is used in a certain time period