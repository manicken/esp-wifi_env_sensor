#pragma once

#include <Arduino.h> // for String
#include <cstdint>
namespace HAL_JSON {

    class HALValue {
    public:
        enum class Type { NONE, UINT, FLOAT };

    private:
        Type type;
        union {
            uint32_t uval;
            float fval;
        };


    public:
        HALValue();
        HALValue(uint32_t v);
        HALValue(float v);

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
