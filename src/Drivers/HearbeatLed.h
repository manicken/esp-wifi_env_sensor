#include <Arduino.h>

#if defined(ESP8266)
#include <ESP8266WebServer.h>
#define DEBUG_UART Serial1
#define WEBSERVER_TYPE ESP8266WebServer
#elif defined(ESP32)
#include "Support/fs_WebServer.h"
#define DEBUG_UART Serial
#define WEBSERVER_TYPE fs_WebServer
#endif

namespace HeartbeatLed
{

#if defined (ESP32WROVER_E_IE)
    #define HEARTBEATLED_PIN 0
    #define HEARTBEATLED_ACTIVESTATE LOW
    #define HEARTBEATLED_INACTIVESTATE HIGH
    #define HEARTBEATLED_DEFAULT_ON_INTERVAL 100
    #define HEARTBEATLED_DEFAULT_OFF_INTERVAL 4000
#elif defined (ESP32DEV)
    #define HEARTBEATLED_PIN 2
    #define HEARTBEATLED_ACTIVESTATE HIGH
    #define HEARTBEATLED_INACTIVESTATE LOW
    #define HEARTBEATLED_DEFAULT_ON_INTERVAL 100
    #define HEARTBEATLED_DEFAULT_OFF_INTERVAL 4000
#elif defined (ESP8266)
    #define HEARTBEATLED_PIN 0
    #define HEARTBEATLED_ACTIVESTATE LOW
    #define HEARTBEATLED_INACTIVESTATE HIGH
    #define HEARTBEATLED_DEFAULT_ON_INTERVAL 100
    #define HEARTBEATLED_DEFAULT_OFF_INTERVAL 4000
#endif

    unsigned long HEARTBEATLED_ON_INTERVAL = HEARTBEATLED_DEFAULT_ON_INTERVAL;
    unsigned long HEARTBEATLED_OFF_INTERVAL = HEARTBEATLED_DEFAULT_OFF_INTERVAL;
    int ledState = LOW;             // ledState used to set the LED
    unsigned long previousMillis = 0;        // will store last time LED was updated
    unsigned long currentMillis = 0;
    unsigned long currentInterval = 0;

    WEBSERVER_TYPE *webserver = nullptr;

    void setup(WEBSERVER_TYPE &srv)
    {
        webserver = &srv;
        webserver->on("/HeartbeatLed/set", []() {
            bool hadAnyArg = false;
            String ret = "";
            if (webserver->hasArg("on")) {
                hadAnyArg = true;
                String onArg = webserver->arg("on");
                if (Convert::isInteger(onArg.c_str()) == false)
                    ret += "ERROR: on argument is not a number<br>";
                else
                {
                    int onValue = atoi(onArg.c_str());
                    HeartbeatLed::HEARTBEATLED_ON_INTERVAL = onValue;
                    ret += "On value set to " + onArg + "<br>";
                }
            }
            if (webserver->hasArg("off")) {
                hadAnyArg = true;
                String offArg = webserver->arg("off");
                if (Convert::isInteger(offArg.c_str()) == false)
                    ret += "ERROR: off argument is not a number<br>";
                else
                {
                    int offValue = atoi(offArg.c_str());
                    HeartbeatLed::HEARTBEATLED_OFF_INTERVAL = offValue;
                    ret += "Off value set to " + offArg + "<br>";
                }
            }
            if (hadAnyArg == false) {
                ret += "Warning: On/Off arguments missing!!!<br>";
            }
            webserver->send(200, "text/html", ret);
        });
        pinMode(HEARTBEATLED_PIN, OUTPUT);
        digitalWrite(HEARTBEATLED_PIN, HEARTBEATLED_INACTIVESTATE);
    }
    void task(void)
    {
        currentMillis = millis();
        currentInterval = currentMillis - previousMillis;
        
        if (ledState == LOW)
        {
            if (currentInterval > HEARTBEATLED_OFF_INTERVAL)
            {
                previousMillis = currentMillis;
                ledState = HIGH;
                digitalWrite(HEARTBEATLED_PIN, HEARTBEATLED_ACTIVESTATE);
            }
        }
        else
        {
            if (currentInterval > HEARTBEATLED_ON_INTERVAL)
            {
                previousMillis = currentMillis;
                ledState = LOW;
                digitalWrite(HEARTBEATLED_PIN, HEARTBEATLED_INACTIVESTATE);
            }
        }
    }
}