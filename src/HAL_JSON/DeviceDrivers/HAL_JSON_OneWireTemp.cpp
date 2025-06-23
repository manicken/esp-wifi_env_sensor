
#include "HAL_JSON_OneWireTemp.h"

namespace HAL_JSON {
    
    //   ██████  ██████   ██████  ██    ██ ██████  
    //  ██       ██   ██ ██    ██ ██    ██ ██   ██ 
    //  ██   ███ ██████  ██    ██ ██    ██ ██████  
    //  ██    ██ ██   ██ ██    ██ ██    ██ ██      
    //   ██████  ██   ██  ██████   ██████  ██      
    
    Device* OneWireTempGroup::Create(const JsonVariant &jsonObj) {
        return new OneWireTempGroup(jsonObj);
    }

    bool OneWireTempGroup::VerifyJSON(const JsonVariant &jsonObj) {
        // the type don't need any failsafe check as that is taken care of outside this class and is allways available
        const char* typeStr = jsonObj["type"].as<const char*>();

        if (strncmp(typeStr, HAL_JSON_TYPE_ONE_WIRE_TEMP_GROUP, 4) == 0) {
            if (jsonObj.containsKey(HAL_JSON_KEYNAME_ITEMS) == false) {
                GlobalLogger.Error(HAL_JSON_ERR_MISSING_KEY(HAL_JSON_KEYNAME_ITEMS));
                return false;
            }
            if (jsonObj[HAL_JSON_KEYNAME_ITEMS].is<JsonArray>() == false) {
                GlobalLogger.Error(HAL_JSON_ERR_VALUE_TYPE(HAL_JSON_KEYNAME_ITEMS " not array"));
                return false;
            }
            const JsonArray items = jsonObj[HAL_JSON_KEYNAME_ITEMS].as<JsonArray>();
            if (items.size() == 0) {
                GlobalLogger.Error(HAL_JSON_ERR_ITEMS_EMPTY("OneWireTempGroup"));
                return false;
            }
            size_t itemCount = items.size();
            size_t validItemCount = 0;
            for (int i=0;i<itemCount;i++) {
                const JsonVariant item = items[i];
                if (item.is<const char*>() == false) continue; // comment item
                if (OneWireBus::VerifyJSON(item) == true) validItemCount++;
            }
            if (validItemCount == 0) {
                GlobalLogger.Error(HAL_JSON_ERR_ITEMS_NOT_VALID("OneWireTempGroup"));
                return false;
            } 
            return true;
        }
        else if (strncmp(typeStr, HAL_JSON_TYPE_ONE_WIRE_TEMP_BUS, 4) == 0) {
            if (jsonObj.containsKey(HAL_JSON_KEYNAME_PIN) == false) {
                GlobalLogger.Error(HAL_JSON_ERR_MISSING_KEY(HAL_JSON_KEYNAME_PIN));
                return false;
            }
            if (jsonObj.containsKey(HAL_JSON_KEYNAME_ITEMS) == false) {
                GlobalLogger.Error(HAL_JSON_ERR_MISSING_KEY(HAL_JSON_KEYNAME_ITEMS));
                return false;
            }
            if (jsonObj[HAL_JSON_KEYNAME_ITEMS].is<JsonArray>() == false) {
                GlobalLogger.Error(HAL_JSON_ERR_VALUE_TYPE(HAL_JSON_KEYNAME_ITEMS " not array"));
                return false;
            }
            return OneWireBus::VerifyJSON(jsonObj);
        }
        else if (strncmp(typeStr, HAL_JSON_TYPE_ONE_WIRE_TEMP_DEVICE, 4) == 0) {
            if (jsonObj.containsKey(HAL_JSON_KEYNAME_PIN) == false) {
                GlobalLogger.Error(HAL_JSON_ERR_MISSING_KEY(HAL_JSON_KEYNAME_PIN));
                return false;
            }
            return OneWireTempDevice::VerifyJSON(jsonObj);
        }
        GlobalLogger.Error(F("DeviceTypesRegistryError"));
        return false;
    }

    OneWireTempGroup::OneWireTempGroup(const JsonVariant &jsonObj) {
        double rawSec = 1.0;
        if (jsonObj.containsKey("refreshtimesec")) {
            JsonVariant rtimeObj = jsonObj["refreshtimesec"];
            if (rtimeObj.is<float>() || rtimeObj.is<double>()) {
                rawSec = rtimeObj.as<double>();
            } else if (rtimeObj.is<const char*>()) {
                rawSec = atof(rtimeObj.as<const char*>());
            } else if (rtimeObj.is<uint32_t>()) {
                rawSec = static_cast<double>(rtimeObj.as<uint32_t>());
            }

            if (rawSec < 1.0) rawSec = 1.0;
        }
        refreshTimeMs = static_cast<uint32_t>(round(rawSec * 1000));

        // will maybe do the following outside before creating new instanse of this class
        // will see if I want to force usage of uid for the group
        // to answer my own question:
        // there could be a situation when only one 1-wire temp device is defined in the devmgr json list
        // in that case the single device will be created with a default OneWireGroup containing one OneWireBus with the device
        // which case that is could be determined by checking the type field of the current input JsonVariant jsonObj
        if (jsonObj.containsKey("uid")) {
            JsonVariant uidObj = jsonObj["uid"];
            if (uidObj.is<const char*>()) {
                const char* uidStr = uidObj.as<const char*>();
                uid = encodeUID(uidStr);
            }
        }

        // the type don't need any failsafe check as that is taken care of outside this class and is allways available
        const char* typeStr = jsonObj["type"].as<const char*>();

        if (strncmp(typeStr, HAL_JSON_TYPE_ONE_WIRE_TEMP_GROUP, 4) == 0) {

        } else if (strncmp(typeStr, HAL_JSON_TYPE_ONE_WIRE_TEMP_DEVICE, 4) == 0) {

        } else if (strncmp(typeStr, HAL_JSON_TYPE_ONE_WIRE_TEMP_BUS, 4) == 0) {
            
        }
    }
    OneWireTempGroup::~OneWireTempGroup() {
        if (busses != nullptr) {
            for (int i=0;i<busCount;i++)
                delete busses[i];
        }
        delete[] busses;
        busses = nullptr;
    }

    bool OneWireTempGroup::read(const HALReadRequest &req) {
        return false;
    }
    bool OneWireTempGroup::write(const HALWriteRequest&req) {
        return false;
    }

    void OneWireTempGroup::loop() {
        uint32_t now = millis();
        switch (state) {
            case IDLE:
                if (now - lastUpdateMs >= refreshTimeMs) {
                    for (int i=0;i<busCount;i++) {
                        busses[i]->requestTemperatures();
                    }
                    state = WAITING_FOR_CONVERSION;
                    lastStart = now;
                }
                break;
            case WAITING_FOR_CONVERSION:
                if (now - lastStart >= refreshTimeMs) {
                    for (int i=0;i<busCount;i++) {
                        busses[i]->readAll();
                    }
                    lastUpdateMs = now;
                    state = IDLE;
                }
                break;
        }
    }

    //  ██████  ██    ██ ███████ 
    //  ██   ██ ██    ██ ██      
    //  ██████  ██    ██ ███████ 
    //  ██   ██ ██    ██      ██ 
    //  ██████   ██████  ███████ 

    bool OneWireBus::VerifyJSON(const JsonVariant &jsonObj) {
        const JsonArray items = jsonObj[HAL_JSON_KEYNAME_ITEMS].as<JsonArray>();
        if (items.size() == 0) { GlobalLogger.Error(HAL_JSON_ERR_ITEMS_EMPTY("OneWireBus")); return false;}
        size_t itemCount = items.size();
        size_t validItemCount = 0;
        for (int i=0;i<itemCount;i++) {
            const JsonVariant item = items[i];
            if (item.is<const char*>() == false) continue; // comment item
            if (OneWireTempDevice::VerifyJSON(item) == true) validItemCount++;
        }
        if (validItemCount == 0) { GlobalLogger.Error(HAL_JSON_ERR_ITEMS_NOT_VALID("OneWireBus")); return false; }
        return true;
    }

    OneWireBus::~OneWireBus() {
        if (devices != nullptr) {
            for (int i=0;i<deviceCount;i++)
                delete devices[i];
        }
        delete[] devices;
        devices = nullptr;
    }

    //  ██████  ███████ ██    ██ ██  ██████ ███████ 
    //  ██   ██ ██      ██    ██ ██ ██      ██      
    //  ██   ██ █████   ██    ██ ██ ██      █████   
    //  ██   ██ ██       ██  ██  ██ ██      ██      
    //  ██████  ███████   ████   ██  ██████ ███████ 

    bool OneWireTempDevice::VerifyJSON(const JsonVariant &jsonObj) {
        if (jsonObj.containsKey(HAL_JSON_KEYNAME_UID) == false){ return HAL_JSON_ERR_MISSING_KEY(HAL_JSON_KEYNAME_UID); }
        if (jsonObj[HAL_JSON_KEYNAME_UID].is<const char*>() == false){ return HAL_JSON_ERR_VALUE_TYPE(HAL_JSON_KEYNAME_UID); }
        if (jsonObj[HAL_JSON_KEYNAME_UID].as<const char*>() == nullptr){ return HAL_JSON_ERR_STRING_EMPTY(HAL_JSON_KEYNAME_UID); }

        if (jsonObj.containsKey(HAL_JSON_KEYNAME_ONE_WIRE_ROMID) == false){ return HAL_JSON_ERR_MISSING_KEY(HAL_JSON_KEYNAME_ONE_WIRE_ROMID); }
        if (jsonObj[HAL_JSON_KEYNAME_ONE_WIRE_ROMID].is<const char*>() == false){ return HAL_JSON_ERR_VALUE_TYPE(HAL_JSON_KEYNAME_ONE_WIRE_ROMID); }
        if (jsonObj[HAL_JSON_KEYNAME_ONE_WIRE_ROMID].as<const char*>() == nullptr){ return HAL_JSON_ERR_STRING_EMPTY(HAL_JSON_KEYNAME_ONE_WIRE_ROMID); }

        return true;
    }

    OneWireTempDevice::~OneWireTempDevice() {
        
    }
}