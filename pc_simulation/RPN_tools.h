
#pragma once

#include <iostream>
#include <cctype>
#include <string>
#include <cstring>
#include <cstdint>

namespace HAL_JSON
{
    bool IsOperator(char c);
    bool ToRPN(const char* tokens[], int tokenCount, const char* outputRPN[], int& outCount);

} // namespace HAL_JSON