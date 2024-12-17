#pragma once

/*
    this is a NTP helper
*/


#include <time.h>
#include <Arduino.h>

#if defined(ESP8266)
    #define DEBUG_UART Serial1
#elif defined(ESP32)
    #define DEBUG_UART Serial
#endif

namespace NTP {
    #define NTP_TIME_ZONE 1

    extern time_t now;
    extern time_t nowish;

    void NTPConnect(void);
}
