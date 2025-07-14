#include "HAL_JSON_Value.h"
namespace HAL_JSON {
    HALValue::HALValue() : type(Type::NONE) {}

    /*HALValue::HALValue(const HALValue& other) {
        type = other.type;
        if (type == Type::STRING && other.str) {
            str = strdup(other.str);
        } else {
            uval = other.uval;
        }
    }*/

    HALValue::HALValue(uint32_t v) : type(Type::UINT), uval(v) {}

    HALValue::HALValue(float v) : type(Type::FLOAT), fval(v) {}

    //HALValue::HALValue(char* str) : type(Type::STRING), str(v) {}

    HALValue::Type HALValue::getType() const {
        return type;
    }

    bool HALValue::isNumber() const {
        return type == Type::UINT || type == Type::FLOAT;
    }

    uint32_t HALValue::asUInt() const {
        return uval;
    }

    float HALValue::asFloat() const {
        return fval;
    }

    String HALValue::toString() const {
        switch (type) {
            case Type::UINT: return String(uval);
            case Type::FLOAT: return String(fval, 2);
            default: return "";
        }
    }

    HALValue& HALValue::operator=(uint32_t v) {
        set(v);
        return *this;
    }
    HALValue& HALValue::operator=(int32_t v) {
        set(v);
        return *this;
    }

    HALValue& HALValue::operator=(float v) {
        set(v);
        return *this;
    }

    HALValue::operator int32_t() const {
        if (type == Type::INT) return uval;
        // Handle error or conversion
        return 0;
    }

    HALValue::operator uint32_t() const {
        if (type == Type::UINT) return uval;
        // Handle error or conversion
        return 0;
    }
    HALValue::operator uint8_t() const {
        if (type == Type::UINT) return uval;
        // Handle error or conversion
        return 0;
    }

    HALValue::operator float() const {
        if (type == Type::FLOAT) return fval;
        // Handle error or conversion
        return 0.0f;
    }

    void HALValue::set(uint32_t v) {
        type = Type::UINT;
        uval = v;
    }

    void HALValue::set(int32_t v) {
        type = Type::INT;
        uval = v;
    }

    void HALValue::set(float v) {
        type = Type::FLOAT;
        fval = v;
    }

}