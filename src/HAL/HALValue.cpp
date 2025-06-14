#include "HALValue.h"
namespace HAL {
    HALValue::HALValue() : type(Type::NONE) {}

    HALValue::HALValue(uint32_t v) : type(Type::UINT), uval(v) {}

    HALValue::HALValue(float v) : type(Type::FLOAT), fval(v) {}

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

    HALValue& HALValue::operator=(float v) {
        set(v);
        return *this;
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

    void HALValue::set(float v) {
        type = Type::FLOAT;
        fval = v;
    }

}