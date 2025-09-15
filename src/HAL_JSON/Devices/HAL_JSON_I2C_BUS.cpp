
#include "HAL_JSON_I2C_BUS.h"

// Available I2C device types here
#include "Display_SSD1306/HAL_JSON_Display_SSD1306.h"

namespace HAL_JSON {

    const I2C_DeviceTypeDef I2C_DeviceRegistry[] = {
        {"SSD1306", Display_SSD1306::Create, Display_SSD1306::VerifyJSON},
        /** mandatory null terminator */
        {nullptr, nullptr, nullptr}
    };
    const I2C_DeviceTypeDef* GetI2C_DeviceTypeDef(const char* type) {
        int i=0;
        while (true) {
            const I2C_DeviceTypeDef& def = I2C_DeviceRegistry[i++];
            if (def.typeName == nullptr) break;
            if (strcasecmp(def.typeName, type) == 0) return &def;
        }
        return nullptr;
    }
    
    I2C_BUS::I2C_BUS(const JsonVariant &jsonObj, const char* type) : Device(UIDPathMaxLength::Many,type) {
        deviceCount = 0;
        devices = nullptr;

        const char* uidStr = GetAsConstChar(jsonObj,HAL_JSON_KEYNAME_UID);
        uid = encodeUID(uidStr);

        sckpin = GetAsUINT8(jsonObj, "sckpin");
        sdapin = GetAsUINT8(jsonObj, "sdapin");
        freq = GetAsUINT32(jsonObj, "freq");
        if (freq < 100000) freq = 100000; // defaults to 100khz

        int busIndex = GetAsUINT8(jsonObj, "busindex");
#if defined(ESP32)
        if (busIndex == 1)
            wire = &Wire1;
        else
#endif
            wire = &Wire;

        wire->begin(sdapin, sckpin, freq);

        const JsonArray items = jsonObj[HAL_JSON_KEYNAME_ITEMS].as<JsonArray>();

        int itemCount = items.size();
        bool* validItems = new bool[itemCount];
        // first pass count valid items
        size_t validItemCount = 0;
        for (int i=0;i<itemCount;i++) {
            const JsonVariant item = items[i];
            if (IsConstChar(item) == true) { validItems[i] = false; continue; }// comment item
            if (Device::DisabledInJson(item) == true) { validItems[i] = false; continue; } // disabled
            if (ValidateJsonStringField(item, HAL_JSON_KEYNAME_TYPE) == false) { validItems[i] = false; continue; }
            
            const char* type = GetAsConstChar(item, HAL_JSON_KEYNAME_TYPE);
            
            const I2C_DeviceTypeDef* def = GetI2C_DeviceTypeDef(type);
            // no nullcheck is needed as ValidateJSON ensures that all types are correct
            if (def->Verify_JSON_Function(item) == false) { validItems[i] = false; continue; }
            validItemCount++;
            validItems[i] = true;
        }
        // second pass actually create the devices
        deviceCount = validItemCount;
        devices = new Device*[validItemCount]();
        int index = 0;
        for (int i=0;i<itemCount;i++) {
            const JsonVariant item = items[i];
            if (validItems[i] == false) continue;
            
            const char* type = GetAsConstChar(item, HAL_JSON_KEYNAME_TYPE);
            const I2C_DeviceTypeDef* def = GetI2C_DeviceTypeDef(type);
             // no nullcheck is needed as ValidateJSON ensures that all types are correct
            devices[index++] = def->Create_Function(item, "SSD1306", *wire);
        }
        delete[] validItems;
    }
    I2C_BUS::~I2C_BUS() {
        if (devices != nullptr) {
            for (int i=0;i<deviceCount;i++) {
                delete devices[i];
                devices[i] = nullptr;
            }
            delete[] devices;
            devices = nullptr;
            deviceCount = 0;
        }
        wire->end();
        pinMode(sckpin, INPUT);
        pinMode(sdapin, INPUT);
    }

    bool I2C_BUS::VerifyJSON(const JsonVariant &jsonObj) {
        if (!GPIO_manager::ValidateJsonAndCheckIfPinAvailableAndReserve(jsonObj, "sckpin", static_cast<uint8_t>(GPIO_manager::PinMode::OUT))) {
            SET_ERR_LOC(HAL_JSON_ERROR_SOURCE_I2C_VERIFY_JSON);
            return false;
        }
        if (!GPIO_manager::ValidateJsonAndCheckIfPinAvailableAndReserve(jsonObj, "sdapin", static_cast<uint8_t>(GPIO_manager::PinMode::OUT) | static_cast<uint8_t>(GPIO_manager::PinMode::IN))) {
            SET_ERR_LOC(HAL_JSON_ERROR_SOURCE_I2C_VERIFY_JSON);
            return false;
        }
        if (jsonObj.containsKey(HAL_JSON_KEYNAME_ITEMS) == false) {
            GlobalLogger.Error(HAL_JSON_ERR_MISSING_KEY(HAL_JSON_KEYNAME_ITEMS));
            SET_ERR_LOC(HAL_JSON_ERROR_SOURCE_I2C_VERIFY_JSON);
            return false;
        }
        if (jsonObj[HAL_JSON_KEYNAME_ITEMS].is<JsonArray>() == false) {
            GlobalLogger.Error(HAL_JSON_ERR_VALUE_TYPE(HAL_JSON_KEYNAME_ITEMS " not array"));
            SET_ERR_LOC(HAL_JSON_ERROR_SOURCE_I2C_VERIFY_JSON);
            return false;
        }
        const JsonArray items = jsonObj[HAL_JSON_KEYNAME_ITEMS].as<JsonArray>();
        if (items.size() == 0) {
            GlobalLogger.Error(HAL_JSON_ERR_ITEMS_EMPTY());
            SET_ERR_LOC(HAL_JSON_ERROR_SOURCE_I2C_VERIFY_JSON);
            return false;
        }
        int itemCount = items.size();
        size_t validItemCount = 0;
        for (int i=0;i<itemCount;i++) {
            const JsonVariant item = items[i];
            if (IsConstChar(item) == true) continue; // comment item
            if (Device::DisabledInJson(item) == true) continue; // disabled
            if (ValidateJsonStringField(item, HAL_JSON_KEYNAME_TYPE) == false) HAL_JSON_VALIDATE_IN_LOOP_FAIL_OPERATION;
            
            const char* type = GetAsConstChar(item, HAL_JSON_KEYNAME_TYPE);
            const I2C_DeviceTypeDef* def = GetI2C_DeviceTypeDef(type);
            if (def == nullptr) {
                GlobalLogger.Error(F("VerifyI2CDeviceJson - could not find type:"),type);
                SET_ERR_LOC(HAL_JSON_ERROR_SOURCE_I2C_VERIFY_JSON);
                return false;
            }
            
            if (def->Verify_JSON_Function == nullptr) {
                GlobalLogger.Error(F("VerifyI2CDeviceJson - Verify_JSON_Function nullptr:"),type);
                SET_ERR_LOC(HAL_JSON_ERROR_SOURCE_I2C_VERIFY_JSON);
                return false;
            }
            if (def->Create_Function == nullptr) {
                GlobalLogger.Error(F("VerifyI2CDeviceJson - Create_Function nullptr:"),type);
                SET_ERR_LOC(HAL_JSON_ERROR_SOURCE_I2C_VERIFY_JSON);
                return false;
            }
            if (def->Verify_JSON_Function(item) == false) HAL_JSON_VALIDATE_IN_LOOP_FAIL_OPERATION;
            validItemCount++;

        }
        if (validItemCount == 0) {
            GlobalLogger.Error(HAL_JSON_ERR_ITEMS_NOT_VALID("I2C_BUS"));
            SET_ERR_LOC(HAL_JSON_ERROR_SOURCE_I2C_VERIFY_JSON);
            return false;
        }
        return true;
    }

    Device* I2C_BUS::Create(const JsonVariant &jsonObj, const char* type) {
        return new I2C_BUS(jsonObj, type);
    }

    String I2C_BUS::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += type;
        ret += ",\"SDA PIN\":";
        ret += std::to_string(sdapin).c_str();
        ret += ",\"SCK PIN\":";
        ret += std::to_string(sckpin).c_str();
        ret += ",\"freq\":";
        ret += std::to_string(freq).c_str();
        ret += ",\"devices\":[";
        for (int i=0;i<deviceCount;i++) {
            ret += "{";
            ret += devices[i]->ToString();
            ret += "}";
            if (i<deviceCount-1) ret += ",";
        }
        ret += "]";
        return ret;
    }

    Device* I2C_BUS::findDevice(UIDPath& path) {
        printf("\nI2C_BUS::findDevice: %s\n", path.ToString().c_str());
        return Device::findInArray(reinterpret_cast<Device**>(devices), deviceCount, path, this);
    }

    void I2C_BUS::loop() {
        for (int i=0;i<deviceCount;i++) {
            devices[i]->loop();
        }
    }

}