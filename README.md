# TECHIN-514_Final

## Project Description: 
This project is a personal desktop weather station, displaying real-time outdoor temperature, humidity, and pressure, with gesture-controlled interactive OLED screen and gauge needle as interface.

## Features:
1. Display real-time outdoor temperature, humidity, and pressure.
2. Use hand gesture to controll the information on the OLED.

## Component Datasheet (Sensors & Displayes)
![GitHub Logo](https://github.com/KrantLeeee/TECHIN-514_Final/blob/main/IMG/image%2055.png)
### How they work
![GitHub Logo](https://github.com/KrantLeeee/TECHIN-514_Final/blob/main/IMG/553ca108202862de61ef6196337a7ab.jpg)

## How They Comminicate
![GitHub Logo](https://github.com/KrantLeeee/TECHIN-514_Final/blob/main/IMG/ARCHITECTURE%201.png)
![GitHub Logo](https://github.com/KrantLeeee/TECHIN-514_Final/blob/main/IMG/ARCHITECTURE%202.png)

##Schematic
![GitHub Logo](https://github.com/KrantLeeee/TECHIN-514_Final/blob/main/IMG/WIRING.png)

##PCB Design
![GitHub Logo](https://github.com/KrantLeeee/TECHIN-514_Final/blob/main/IMG/PCB.png)

##DSP
A moving average filter is used to ensure accurate and stable readings.
The filter smooths out transient noise and mitigates anomalous data.
This improves the reliability of the information provided by the weather station.

##Function Logic
Placement and Power
The sensor device is positioned on an outdoor balcony.
It's powered via USB, eliminating the need for frequent battery replacements.

Data Transmission
Bluetooth is used to bridge the data gap between the sensor and the display.

Display Device Features
Displays real-time readings on an OLED screen.
Uses a step motor-driven dashboard to interpret and visually indicate humidity levels, providing an intuitive humidity indicator.
Operated on a 2000mAh battery for portability and convenience.
Incorporates gesture control via HiLetgo APDS-9960 for toggling displayed metrics, enhancing user interaction without cluttering the device's aesthetic.
