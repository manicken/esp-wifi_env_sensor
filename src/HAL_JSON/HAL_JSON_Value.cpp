#include "HAL_JSON_Value.h"
namespace HAL_JSON {
    HALValue::HALValue() : type(Type::UNSET) {}

    /*HALValue::HALValue(const HALValue& other) {
        type = other.type;
        if (type == Type::STRING && other.str) {
            str = strdup(other.str);
        } else {
            uval = other.uval;
        }
    }*/
    HALValue::HALValue(int32_t v) : type(Type::INT), ival(v) {}

    HALValue::HALValue(uint32_t v) : type(Type::UINT), uval(v) {}

    HALValue::HALValue(float v) : type(Type::FLOAT), fval(v) {}

    HALValue::HALValue(bool v) : type(Type::UINT), uval(v?1:0) {}

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

    int32_t HALValue::asInt() const {
        return ival;
    }

    float HALValue::asFloat() const {
        if (type == Type::FLOAT)
            return fval;
        else if (type == Type::INT)
            return static_cast<float>(ival);
        else if (type == Type::UINT)
            return static_cast<float>(uval);
        else
            return 0.0f;
    }

    std::string HALValue::toString() const {
        switch (type) {
            case Type::INT: return std::to_string(ival);
            case Type::UINT: return std::to_string(uval);
            case Type::FLOAT: return std::to_string(fval);
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



    bool operator==(const HALValue& lhs, const HALValue& rhs) {
        return lhs.asFloat() == rhs.asFloat();
    }

    bool operator!=(const HALValue& lhs, const HALValue& rhs) {
        return !(lhs == rhs);
    }

    bool operator<(const HALValue& lhs, const HALValue& rhs) {
        return lhs.asFloat() < rhs.asFloat();
    }

    bool operator>(const HALValue& lhs, const HALValue& rhs) {
        return lhs.asFloat() > rhs.asFloat();
    }

    bool operator<=(const HALValue& lhs, const HALValue& rhs) {
        return lhs.asFloat() <= rhs.asFloat();
    }

    bool operator>=(const HALValue& lhs, const HALValue& rhs) {
        return lhs.asFloat() >= rhs.asFloat();
    }

    HALValue HALValue::operator+(const HALValue& other) const {
        if (type == Type::FLOAT || other.type == Type::FLOAT) {
            return asFloat() + other.asFloat();
        } else if (type == Type::INT || other.type == Type::INT) {
            return ival + other.ival;
        } else {
            return uval + other.uval;
        }
    }
    HALValue HALValue::operator-(const HALValue& other) const {
        if (type == Type::FLOAT || other.type == Type::FLOAT) {
            return asFloat() - other.asFloat();
        } else if (type == Type::INT || other.type == Type::INT) {
            return ival - other.ival;
        } else {
            if (other.uval <= uval)
                return uval - other.uval;
            else
                return ival - other.ival;
        }
    }
    HALValue HALValue::operator*(const HALValue& other) const {
        if (type == Type::FLOAT || other.type == Type::FLOAT) {
            return asFloat() * other.asFloat();
        } else if (type == Type::INT || other.type == Type::INT) {
            return ival * other.ival;
        } else {
            return uval * other.uval;
        }
    }
    HALValue HALValue::operator/(const HALValue& other) const {
        if (type == Type::FLOAT || other.type == Type::FLOAT) {
            return asFloat() / other.asFloat();
        } else if (type == Type::INT || other.type == Type::INT) {
            return ival / other.ival;
        } else {
            return uval / other.uval;
        }
    }
    HALValue HALValue::operator%(const HALValue& other) const {
        return uval % other.uval;
    }
    HALValue HALValue::operator&(const HALValue& other) const {
        return uval & other.uval;
    }
    HALValue HALValue::operator|(const HALValue& other) const {
        return uval | other.uval;
    }
    HALValue HALValue::operator^(const HALValue& other) const {
        return uval ^ other.uval;
    }
    HALValue HALValue::operator<<(const HALValue& other) const {
        return uval << other.uval;
    }
    HALValue HALValue::operator>>(const HALValue& other) const {
        return uval >> other.uval;
    }

}