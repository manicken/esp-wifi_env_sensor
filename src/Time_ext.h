#pragma once

#include <TimeLib.h>
#include <stdlib.h>
#include <string>

namespace Time_ext
{
    std::string formatNumber(int num);
    std::string formatTime(int hour, int minute, int second);
    std::string GetTimeAsString(time_t time);
}