
#include <TimeLib.h>
#include <stdlib.h>
#include <string>
#include "Time_ext.h"

namespace Time_ext
{
    std::string formatNumber(int num) {
        return (num < 10) ? "0" + std::to_string(num) : std::to_string(num);
    }

    std::string formatTime(int hour, int minute, int second) {
        return formatNumber(hour) + ":" + formatNumber(minute) + ":" + formatNumber(second);
    }

    std::string GetTimeAsString(time_t time)
    {
        tmElements_t now2;
        breakTime(time, now2);
        std::string nowstr = std::to_string(now2.Year + 1970) + "-" +
                        std::to_string(now2.Month) + "-" +
                        std::to_string(now2.Day) + " " +
                        formatTime(now2.Hour, now2.Minute, now2.Second);
                        
        return nowstr;
    }
}