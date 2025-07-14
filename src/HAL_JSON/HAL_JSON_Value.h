#pragma once

#include <Arduino.h> // for String
#include <cstdint>
namespace HAL_JSON {

    class HALValue {
    public:
        enum class Type { NONE, UINT, INT, FLOAT/*, STRING*/ };

    private:
        Type type;
        union {
            uint32_t uval;
            float fval;
            //char* str; // future use?? // check chatgpt conversation "Function Parameter Passing"
            //uint8_t* arr; // future use (here the first byte defines how many lenght def bytes that comes after, and then that lenght def bytes def. how many actual data bytes come after that)
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

        void set(int32_t v);
        void set(uint32_t v);
        void set(float v);

        HALValue& operator=(int32_t v);
        HALValue& operator=(uint32_t v);
        HALValue& operator=(float v);

        operator uint8_t() const;
        operator uint32_t() const;
        operator int32_t() const;
        operator float() const;
    };
}
