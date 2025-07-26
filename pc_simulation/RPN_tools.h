
#pragma once

#ifndef _WIN32
#include <Arduino.h>
#else

#endif
#include <iostream>
#include <cctype>
#include <string>
#include <cstring>
#include <cstdint>

namespace HAL_JSON
{
    bool ToRPN(const char* tokens[], int tokenCount, const char* outputRPN[], int& outCount);

} // namespace HAL_JSON