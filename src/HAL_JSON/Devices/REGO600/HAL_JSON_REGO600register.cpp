
#include "HAL_JSON_REGO600register.h"

namespace HAL_JSON {
    
    REGO600register::REGO600register(const JsonVariant &jsonObj, const char* type) : Device(UIDPathMaxLength::One,type) {
        const char* uidStr = jsonObj[HAL_JSON_KEYNAME_UID].as<const char*>();
        uid = encodeUID(uidStr);
        const char* valueTypeStr = GetAsConstChar(jsonObj, HAL_JSON_KEYNAME_REGO600_VALUE_TYPE);
        
        if (CharArray::equalsIgnoreCase(valueTypeStr, "float"))
            valueType = ValueType::FLOAT;
        else if (CharArray::equalsIgnoreCase(valueTypeStr, "uint"))
            valueType = ValueType::UINT;
        else if (CharArray::equalsIgnoreCase(valueTypeStr, "int"))
            valueType = ValueType::INT;
        else if (CharArray::equalsIgnoreCase(valueTypeStr, "bool"))
            valueType = ValueType::BOOL;
        //else this will never happen if VerifyJSON is used beforehand
        
        // special note here
        // HAL_JSON_REGO600_KEYNAME_OPCODE and HAL_JSON_REGO600_KEYNAME_ADDRESS
        // are extracted outside of this class instance as they are used elsewhere
    }

    bool REGO600register::VerifyJSON(const JsonVariant &jsonObj) {
        if (ValidateJsonStringField(jsonObj, HAL_JSON_KEYNAME_UID) == false) { return false; }
        if (ValidateJsonStringField(jsonObj, HAL_JSON_KEYNAME_REGO600_OPCODE) == false) { return false; }
        if (ValidateJsonStringField(jsonObj, HAL_JSON_KEYNAME_REGO600_ADDRESS) == false) { return false; }
        const char* opcodeStr = GetAsConstChar(jsonObj, HAL_JSON_KEYNAME_REGO600_OPCODE);
        if (!Convert::IsValidHexString(opcodeStr)) { GlobalLogger.Error(F("REGO600regItem - not valid hexstr: "), opcodeStr); return false; }
        const char* addressStr = GetAsConstChar(jsonObj, HAL_JSON_KEYNAME_REGO600_ADDRESS);
        if (!Convert::IsValidHexString(addressStr)) { GlobalLogger.Error(F("REGO600regItem - not valid hexstr: "), addressStr); return false; }

        if (ValidateJsonStringField(jsonObj, HAL_JSON_KEYNAME_REGO600_VALUE_TYPE) == false) { return false; }
        const char* valueTypeStr = GetAsConstChar(jsonObj, HAL_JSON_KEYNAME_REGO600_VALUE_TYPE);
        bool validValueType = false;
        if (CharArray::equalsIgnoreCase(valueTypeStr, "float")) validValueType = true;
        else if (CharArray::equalsIgnoreCase(valueTypeStr, "uint")) validValueType = true;
        else if (CharArray::equalsIgnoreCase(valueTypeStr, "int")) validValueType = true;
        else if (CharArray::equalsIgnoreCase(valueTypeStr, "bool")) validValueType = true;
        if (validValueType == false) { GlobalLogger.Error(F("REGO600regItem - valueType invalid"), valueTypeStr); return false; }
        return true;
    }

    Device* REGO600register::Create(const JsonVariant &jsonObj, const char* type) {
        return new REGO600register(jsonObj, type);
    }

    bool REGO600register::read(HALValue& val) {
        if (valueType == ValueType::FLOAT)
            val = (float)((float)value / 10.0f);
        else if (valueType == ValueType::BOOL)
            val = (uint32_t)((value > 0)?1:0);
        else if (valueType == ValueType::UINT)
            val = value;
        else if (valueType == ValueType::INT)
            val = (int32_t)value;
        else {
            return false; // should not happen
        }
        return true;
    }

    String REGO600register::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\"";
        return ret;
    }

}