
#include "HAL_JSON_REGO600.h"

namespace HAL_JSON {
    
    REGO600::REGO600(const JsonVariant &jsonObj, const char* type) : Device(UIDPathMaxLength::One,type) {

        JsonArray items = jsonObj[HAL_JSON_KEYNAME_ITEMS];
        size_t itemCount = items.size();
        bool* validItems = new bool[itemCount];
        registerItemCount = 0;
        // first pass count valid items
        for (int i=0;i<itemCount;i++) {


        }
        // second pass
        requestList = new (std::nothrow) Drivers::REGO600::Request*[registerItemCount];
        registerItems = new (std::nothrow) REGO600register*[registerItemCount];
        int index = 0;
        for (int i=0;i<itemCount;i++) {
            const JsonVariant& item = items[i];
            registerItems[index] = new REGO600register(item, nullptr);
            const char* opcodeStr = GetAsConstChar(jsonObj, HAL_JSON_KEYNAME_REGO600_OPCODE);
            const char* addressStr = GetAsConstChar(jsonObj, HAL_JSON_KEYNAME_REGO600_ADDRESS);
            uint32_t opcode = std::strtoul(opcodeStr, nullptr, 16);
            uint16_t address = std::strtoul(addressStr, nullptr, 16);
            // here value is passed by ref so that REGO600 driver can access and change the value so REGO600register read function can then get the correct value
            requestList[index] = new Drivers::REGO600::Request(opcode, address, registerItems[i]->value); 
            index++;
        }
        rego600 = new Drivers::REGO600(rxPin, txPin, requestList, registerItemCount);
    }

    REGO600::~REGO600() {
        if (rego600 != nullptr)
            delete rego600;
        if (requestList != nullptr) { // if for example the allocation did fail
            for (auto i=0;i<registerItemCount; i++) {
                delete requestList[i];
            }
            delete[] requestList;
        }
        if (registerItems != nullptr) { // if for example the allocation did fail
            for (auto i=0;i<registerItemCount; i++) {
                delete registerItems[i];
            }
            delete[] registerItems;
        }
        pinMode(rxPin, INPUT);
        pinMode(txPin, INPUT);
    }
    void REGO600::begin() {
        rego600->begin(); // this will initialize a first request
    }
    void REGO600::loop() {
        rego600->loop();
    }

    bool REGO600::VerifyJSON(const JsonVariant &jsonObj) {
        if (!ValidateUINT8(jsonObj,HAL_JSON_KEYNAME_RXPIN)) return false;
        if (!ValidateUINT8(jsonObj,HAL_JSON_KEYNAME_TXPIN)) return false;
        uint8_t rxPin = GetAsUINT8(jsonObj, HAL_JSON_KEYNAME_RXPIN);
        if (!GPIO_manager::CheckIfPinAvailableAndReserve(rxPin, static_cast<uint8_t>(GPIO_manager::PinMode::IN))) return false;
        uint8_t txPin = GetAsUINT8(jsonObj, HAL_JSON_KEYNAME_TXPIN);
        if (!GPIO_manager::CheckIfPinAvailableAndReserve(txPin, static_cast<uint8_t>(GPIO_manager::PinMode::OUT))) return false;
        if (!jsonObj.containsKey(HAL_JSON_KEYNAME_ITEMS)) { GlobalLogger.Error(HAL_JSON_ERR_MISSING_KEY(HAL_JSON_KEYNAME_ITEMS)); return false; }
        
        if (jsonObj[HAL_JSON_KEYNAME_ITEMS].is<JsonArray>() == false) {
            GlobalLogger.Error(HAL_JSON_ERR_VALUE_TYPE(HAL_JSON_KEYNAME_ITEMS " not array"));
            return false;
        }
        const JsonArray& items = jsonObj[HAL_JSON_KEYNAME_ITEMS].as<JsonArray>();
        if (items.size() == 0) { GlobalLogger.Error(HAL_JSON_ERR_ITEMS_EMPTY("REGO600")); return false;}
        size_t itemCount = items.size();
        size_t validItemCount = 0;
        for (int i=0;i<itemCount;i++) {
            const JsonVariant item = items[i];
            if (IsConstChar(item) == true) continue; // comment item
            if (Device::DisabledInJson(item) == true) continue; // disabled
            if (REGO600register::VerifyJSON(item) == false) HAL_JSON_VALIDATE_IN_LOOP_FAIL_OPERATION;
            validItemCount++;
        }
        if (validItemCount == 0) { GlobalLogger.Error(HAL_JSON_ERR_ITEMS_NOT_VALID("REGO600")); return false; }
        return true;
    }

    Device* REGO600::Create(const JsonVariant &jsonObj, const char* type) {
        return new REGO600(jsonObj, type);
    }

    String REGO600::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += type;
        ret += "\"";
        return ret;
    }

}