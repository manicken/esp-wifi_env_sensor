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

    bool HexToBytes(const char* hexString, uint8_t* byteArray, size_t arraySize)
    {
        if (!hexString) { // if byteArray is nullptr then this function is only used to verify a hexString
            GlobalLogger.Error(F("HexToBytes - Invalid input 1"));
            DEBUG_UART.println(F("HexToBytes - Invalid input 1"));
            return false; // Invalid input 1
        }
        size_t hexStrLen = strlen(hexString);
        int incr = 0;
        if (hexStrLen == arraySize*2) // no deliminator between hex-byte numbers
            incr = 2;
        else if (hexStrLen == (arraySize*3)-1)// any desired deliminator between hex-byte numbers
            incr = 3;
        else {
            GlobalLogger.Error(F("HexToBytes - hexStrLen mismatch:"),String(hexStrLen).c_str());
            DEBUG_UART.print(F("HexToBytes - hexStrLen mismatch:"));
            DEBUG_UART.println(String(hexStrLen));
            return false; // Invalid input 2
        }
        size_t byteIndex = 0;
        for (size_t i = 0; i < hexStrLen; i+=incr) {
            if (byteIndex >= arraySize) {
                GlobalLogger.Error(F("HexToBytes - Exceeded array size"));
                DEBUG_UART.println(F("HexToBytes - Exceeded array size"));
                return false; // Exceeded array size
            }
            char highNibble = hexString[i];
            char lowNibble = hexString[i + 1];

            if (!std::isxdigit(highNibble) || !std::isxdigit(lowNibble)) {
                GlobalLogger.Error(F("HexToBytes - Non-hex character found"));
                DEBUG_UART.println(F("HexToBytes - Non-hex character found"));
                return false; // Non-hex character found
            }
            if (byteArray != nullptr) // if byteArray is nullptr then this is only a convert pre-validate
                byteArray[byteIndex++] = (uint8_t)((std::isdigit(highNibble) ? highNibble - '0' : std::toupper(highNibble) - 'A' + 10) << 4 |
                                    (std::isdigit(lowNibble) ? lowNibble - '0' : std::toupper(lowNibble) - 'A' + 10));
        }

        return true; // Conversion successful
    }

    uint64_t reverseMACaddress(uint64_t addr)
    {
        uint8_t* macAddrPtr = reinterpret_cast<uint8_t*>(&addr);
        uint64_t macAddrBigEndian = (macAddrPtr[0] * 0x10000000000) + 
                                    (macAddrPtr[1] * 0x00100000000) + 
                                    (macAddrPtr[2] * 0x00001000000) + 
                                    (macAddrPtr[3] * 0x00000010000) + 
                                    (macAddrPtr[4] * 0x00000000100) + 
                                    (macAddrPtr[5]);
        return macAddrBigEndian;
    }

}