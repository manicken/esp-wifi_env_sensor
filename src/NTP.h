/*
    this is a NTP helper
*/
#ifndef NTP_H
#define NTP_H

#include <time.h>
#include <Arduino.h>


namespace NTP {
    #define TIME_ZONE 1
    
#if defined(ESP8266)
    #define DEBUG_UART Serial1
#elif defined(ESP32)
    #define DEBUG_UART Serial
#endif

    time_t now;
    time_t nowish = 1510592825;

    void NTPConnect(void)
    {
        DEBUG_UART.print("Setting time using SNTP  ");
        configTime(TIME_ZONE * 3600, 0 * 3600, "pool.ntp.org", "time.nist.gov");
        now = time(nullptr);
        
        while (now < nowish)
        {
            delay(500);
            DEBUG_UART.print(".");
            now = time(nullptr);
        }
        DEBUG_UART.println("[OK]");
        struct tm* timeinfo;
        timeinfo = localtime(&now);
        //gmtime_r(&now, &timeinfo);
        DEBUG_UART.print("Current time: ");
        DEBUG_UART.print(asctime(timeinfo));
        DEBUG_UART.println();
        DEBUG_UART.println();
    }
}
#endif