
#include "NTP.h"

namespace NTP {

    time_t now;
    time_t nowish = 1510592825;

    void NTPConnect(void)
    {
        DEBUG_UART.print("Setting time using SNTP  ");
        configTime(NTP_TIME_ZONE * 3600, 0 * 3600, "pool.ntp.org", "time.nist.gov");
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