
#include "NTP.h"

namespace NTP {

    time_t now;

    void NTPConnect() {
        DEBUG_UART.print("Setting time using SNTP  ");
        configTime(NTP_TIME_ZONE * 3600, 0, "pool.ntp.org", "time.nist.gov");

        const unsigned long ntpTimeout = 30000; // 30 seconds max wait
        unsigned long start = millis();
        time_t now = time(nullptr);
        time_t nowish = 1609459200; // e.g., Jan 1 2021, safe threshold

        while (now < nowish) {
            if (millis() - start > ntpTimeout) {
                DEBUG_UART.println("\nNTP timeout, continuing without valid time");
                break;
            }
            delay(500);
            DEBUG_UART.print(".");
            now = time(nullptr);
        }

        if (now >= nowish) {
            DEBUG_UART.println("[OK]");
            struct tm* timeinfo = localtime(&now);
            DEBUG_UART.print("Current time: ");
            DEBUG_UART.println(asctime(timeinfo));
        } else {
            DEBUG_UART.println("[NO VALID TIME]");
        }
    }

}