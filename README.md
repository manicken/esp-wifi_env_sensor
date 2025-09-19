This project provides a flexible hardware and device abstraction framework for ESP32, ESP8266, Raspberry Pi Pico, and Linux.

* JSON-based configuration: Easily define hardware and virtual devices.

* Hybrid HAL/DAL system: Supports direct hardware access (GPIO, ADC, I²C, SPI, PWM) and logical/virtual devices.

* Script engine: Compile scripts to fast indirect dispatch with function pointers and device contexts.

* Cross-platform: Runs on microcontrollers and Linux, enabling unified device management and IoT integration (e.g., Thingspeak uploads).

Ideal for building modular, multi-device IoT projects and automation systems with minimal platform-specific code.

# License

This repository uses a mixed-license approach:

- **GPL v3** for the core HAL/DAL and script engine files.  
- **MIT** for device registration files:  
  - HAL_JSON_DeviceTypesRegistry.cpp  
  - HAL_JSON_I2C_BUS_DeviceTypeReg.cpp  

Please consult the respective LICENSE files before using or redistributing the code.

currently a work in progress
but when finished:
It will be a better variant to tasmota and espeasy,
specially when using sensors
as the codebase is much easier to understand,
and is easier to add functionality.

The proposed future projectname will be WALHALLA
as it's WiFi/Wireless and HAL 
the HAL abstraction configuration is json based, 
but will in the future be using a 'compiled JSON' - BSON for easier parsing


**dependencies**<br>
special version of TimeAlarms<br>
that can have any amount of alarms (up to 255)<br>
and also can use parameters when calling handler functions<br>
note. this dependency is included in platformio.ini and is automatically installed<br>
https://github.com/manicken/TimeAlarms <br>
