# Arduino-Pebble-Android

## CIT 595 Project

### This project consists of three major components:
- Sensor and Display: a Gravitech 7-Segment Shield attached to an Arduino microcontroller. This component includes a temperature sensor, a seven-segment display (like a digital clock), and an RGB light.

- Middleware: The Arduino is connected via USB to a Linux or Mac machine, which runs a C++ program that handles all communication between the sensor and the user interface.

- User interface: a Pebble smartwatch application, which communicates with the middleware over the Internet via an Android or iOS phone.


### System Features:
* Query temperature with Pebble watch
* Interact and control Sensor with Pebble watch
* Arduino display temperature in Fahrenheit/Celsius
* View temperature history with Pebble watch, displayed in line chart
* Loop up Yahoo stock price and show on both PebbleWatch and Ardunio
* Query Philly weather report from Pebble watch

