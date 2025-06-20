#pragma once

#include <Arduino.h> // for String
#include <cstdint>
namespace HAL_JSON {

    class HALValue {
    public:
        enum class Type { NONE, UINT, FLOAT/*, STRING*/ };

    private:
        Type type;
        union {
            uint32_t uval;
            float fval;
            //char* str; // future use?? // check chatgpt conversation "Function Parameter Passing"
        };


    public:
        HALValue();
        //HALValue(const HALValue& other);  // future use?? only used for string
        HALValue(uint32_t v);
        HALValue(float v);
        // HALValue(char* str); // future use??

        Type getType() const;
        bool isNumber() const;

        uint32_t asUInt() const;
        float asFloat() const;

        String toString() const;

        void set(uint32_t v);
        void set(float v);

        HALValue& operator=(uint32_t v);
        HALValue& operator=(float v);

        operator uint8_t() const;
        operator uint32_t() const;
        operator float() const;
    };
}
