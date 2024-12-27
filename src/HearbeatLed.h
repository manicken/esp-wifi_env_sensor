#include <Arduino.h>

namespace HeartbeatLed
{
    #define HEARTBEATLED_ACTIVESTATE LOW
    #define HEARTBEATLED_INACTIVESTATE HIGH
    #define HEARTBEATLED_PIN 0
    #define HEARTBEATLED_ON_INTERVAL 100
    #define HEARTBEATLED_OFF_INTERVAL 4000
    int ledState = LOW;             // ledState used to set the LED
    unsigned long previousMillis = 0;        // will store last time LED was updated
    unsigned long currentMillis = 0;
    unsigned long currentInterval = 0;

    void init()
    {
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