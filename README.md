# esp-temperature2thingsspeak
WiFi esp based temperature/hygrometer sensor using thingsspeak for data storage

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
