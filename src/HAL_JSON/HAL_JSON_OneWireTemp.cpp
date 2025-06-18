
#include "HAL_JSON_OneWireTemp.h"

namespace HAL_JSON {
    
    //   ██████  ██████   ██████  ██    ██ ██████  
    //  ██       ██   ██ ██    ██ ██    ██ ██   ██ 
    //  ██   ███ ██████  ██    ██ ██    ██ ██████  
    //  ██    ██ ██   ██ ██    ██ ██    ██ ██      
    //   ██████  ██   ██  ██████   ██████  ██      
    
    Device* OneWireTempGroup::Create(JsonVariant &jsonObj) {
        return new OneWireTempGroup(jsonObj);
    }

    HAL_JSON_VERIFY_JSON_RETURN_TYPE OneWireTempGroup::VerifyJSON(JsonVariant &jsonObj) {
        // the type don't need any failsafe check as that is taken care of outside this class and is allways available
        const char* typeStr = jsonObj["type"].as<const char*>();

        if (strncmp(typeStr, HAL_JSON_TYPE_ONE_WIRE_TEMP_GROUP, 4) == 0) {
            return HAL_JSON_VERIFY_JSON_RETURN_OK;
        } else if (strncmp(typeStr, HAL_JSON_TYPE_ONE_WIRE_TEMP_DEVICE, 4) == 0) {
            return HAL_JSON_VERIFY_JSON_RETURN_OK;
        } else if (strncmp(typeStr, HAL_JSON_TYPE_ONE_WIRE_TEMP_BUS, 4) == 0) {
            return HAL_JSON_VERIFY_JSON_RETURN_OK;
        }
        return F("error");
    }

    OneWireTempGroup::OneWireTempGroup(JsonVariant &jsonObj) {
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
        delete[] busses;
        busses = nullptr;
    }

    void OneWireTempGroup::loop() {
        uint32_t now = millis();
        switch (state) {
            case IDLE:
                if (now - lastUpdateMs >= refreshTimeMs) {
                    for (int i=0;i<busCount;i++) {
                        busses[i].requestTemperatures();
                    }
                    state = WAITING_FOR_CONVERSION;
                    lastStart = now;
                }
                break;
            case WAITING_FOR_CONVERSION:
                if (now - lastStart >= refreshTimeMs) {
                    for (int i=0;i<busCount;i++) {
                        busses[i].readAll();
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



    //  ██████  ███████ ██    ██ ██  ██████ ███████ 
    //  ██   ██ ██      ██    ██ ██ ██      ██      
    //  ██   ██ █████   ██    ██ ██ ██      █████   
    //  ██   ██ ██       ██  ██  ██ ██      ██      
    //  ██████  ███████   ████   ██  ██████ ███████ 
}