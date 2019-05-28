# eGreenHouse

eGreenhouse is a lightweight and low-cost greenhouse sensor package integrated with the OPEnS HyperRail. The sensors travel through the greenhouse and transmit a live stream of data to Google Sheets.

## Table of Contents

* Overview
* Setup and Installation
* Contributing
  * Git Workflow

## Overview

eGreenhouse has 3 main components:

* The eGreenhouse sensor package
  * Adafruit Feather M0 Lora + Adalogger?
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

## Contributing

### Git Workflow

Add a detailed git workflow here.
