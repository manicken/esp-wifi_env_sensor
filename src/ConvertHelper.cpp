#include <Arduino.h>
#include "ConvertHelper.h"

namespace Convert
{
    std::string floatToString(float value) {
        char buffer[32]; // Adjust size based on needs
        snprintf(buffer, sizeof(buffer), "%.6f", value); // Format as float with 6 decimals

        std::string result(buffer);

        // Remove trailing zeros
        result.erase(result.find_last_not_of('0') + 1, std::string::npos);
        if (result.back() == '.') {
            result.pop_back(); // Remove decimal point if no fractional part
        }

        return result;
    }

    bool isInteger(const char* str)
    {
        if (str == nullptr || *str == '\0') return false; // Null or empty string is not valid

        // Handle leading sign
        if (*str == '+' || *str == '-') str++;

        // Check the rest of the string
        while (*str != '\0')
        {
            if (!isdigit(*str)) return false;
            str++;
        }

        return true; // All characters are digits
    }

    char ConvertOneNibble(uint8_t value)
    {
        if (value>9) return (value - 10) + 'A';
        else return value + '0';
    }

    std::string ByteToHexString(uint8_t value)
    {
        std::string hexString(2,'0');
        hexString[0] = ConvertOneNibble((value >> 4) & 0x0F);
        hexString[1] = ConvertOneNibble(value & 0x0F);
        return hexString;
    }

    std::string ByteArrayToString(uint8_t* byteArray, size_t arraySize)
    {
        std::string str = "";
        for (int i=0;i<arraySize;i++) {
            str.append(ByteToHexString(byteArray[i]));
            if (i<(arraySize-1)) str.append(":");
        }
        return str;
    }

    bool convertHexToBytes(const char* hexString, uint8_t* byteArray, size_t arraySize)
    {
        if (!hexString || !byteArray ) {
            DEBUG_UART.println("convertHexToBytes - Invalid input 1");
            return false; // Invalid input 1
        }
        size_t hexStrLen = strlen(hexString);
        int incr = 0;
        if (hexStrLen == arraySize*2) // no deliminator between hex-byte numbers
            incr = 2;
        else if (hexStrLen == (arraySize*3)-1)// any desired deliminator between hex-byte numbers
            incr = 3;
        else {
            DEBUG_UART.println("convertHexToBytes - hexStrLen mismatch:" + String(hexStrLen));
            return false; // Invalid input 2
        }
        size_t byteIndex = 0;
        for (size_t i = 0; i < hexStrLen; i+=incr) {
            if (byteIndex >= arraySize) {
                DEBUG_UART.println("Exceeded array size");
                return false; // Exceeded array size
            }
            char highNibble = hexString[i];
            char lowNibble = hexString[i + 1];

            if (!std::isxdigit(highNibble) || !std::isxdigit(lowNibble)) {
                DEBUG_UART.println("Non-hex character found");
                return false; // Non-hex character found
            }

            byteArray[byteIndex++] = (uint8_t)((std::isdigit(highNibble) ? highNibble - '0' : std::toupper(highNibble) - 'A' + 10) << 4 |
                                    (std::isdigit(lowNibble) ? lowNibble - '0' : std::toupper(lowNibble) - 'A' + 10));
        }

        return true; // Conversion successful
    }

}