# eGreenHouse

eGreenhouse is a lightweight and low-cost greenhouse sensor package integrated with the OPEnS HyperRail. The sensors travel through the greenhouse and transmit a live stream of data to Google Sheets.

## Table of Contents

* Overview
* Setup and Installation
 * Dependencies
* Contributing
  * Git Workflow

## Overview

eGreenhouse has 3 main components:

* The eGreenhouse sensor package
  * Adafruit Feather M0 Lora + Adalogger + RTC
  * K30 CO2 sensor
  * TSL2561 luminosity sensor
  * SHT31-D temperature + relative humidity sensor
  * SDS-011 particle sensor (dust/pollen) (Optional: Uses a significant amount of power.)

* The HyperRail hub - Adafruit Feather M0 Basic + nRF shield. There is a separate repository for the HyperRail: https://github.com/OPEnSLab-OSU/HyperRail

* The Ethernet hub - Adafruit Feather Lora + Adafruit Ethernet Feather Wing

The graphical user interface (GUI) which runs in the program, Processing.

We recommend using our custom PCB to to simplify wiring.

(Note: The current version of the custom PCB has an extraneous trace that needs to be broken in order for the board to function properly.)

## Software Setup and Installation
1. Install eGreenhouseSensorsLoRa.ino onto the eGreenhouse sensor package.
2. Install ethernetHubLora.ino onto the Ethernet hub.
3. Install hyperSenseLora.ino onto the HyperRail hub.

### Dependencies
You will need the following libraries and versions to compile code for the eGreenhouse and HyperRail hub (dependencies for Ethernet Hub coming soon). The new versions of these libraries need to be tested.

* Adafruit SHT31 Library 1.0
* Adafruit TSL2561 1.0.2
* Adafruit Unified Sensor 1.0.2
* Adafruit_ZeroDMA 1.0.4
* [OSC](https://github.com/CNMAT/OSC) 1.3.5
* [RadioHead](http://www.airspayce.com/mikem/arduino/RadioHead/)
* RTClib 1.2.0
* SD 1.2.3 - Built in
* SDS011-select-serial-master 0.0.6
* SPI 1.0
* Wire version 1.0

## Contributing

### Git Workflow

Add a detailed git workflow here.
