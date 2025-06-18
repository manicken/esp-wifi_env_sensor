
#include "HAL_JSON.h"

namespace HAL_JSON {
    Device **devices = nullptr;
    uint32_t deviceCount = 0;

    Device* CreateDeviceFromJSON(JsonVariant &jsonObj) {
        const char* type = jsonObj[HAL_JSON_KEYNAME_TYPE].as<const char*>();
        for (int i=0;DeviceRegistry[i].type != nullptr;i++) {
            if (strcmp(type, DeviceRegistry[i].type) == 0) {
                //if (DeviceRegistry[i].Verify_JSON_Function == nullptr) return nullptr; // silenty skip this for now (it do actually mean the list is done but not the implementation)
                if (DeviceRegistry[i].Create_Function == nullptr) return nullptr; // silenty skip this for now (it do actually mean the list is done but not the implementation) also no point of verifying the json if the create function don't exists
                //auto err = DeviceRegistry[i].Verify_JSON_Function(jsonObj);
                //if (err != HAL_JSON_VERIFY_JSON_RETURN_OK) {
                //    Serial.println(err); // just print to the serial for now
                //    return nullptr;
                //} 
                // note to the above commennted out code:
                // it should not be nessesary to do VerifyJson again as it's done in the create all devices loop

                return DeviceRegistry[i].Create_Function(jsonObj);
            }
        }
        return nullptr; // no match
    }
    bool VerifyDeviceJson(JsonVariant &jsonObj) {
        if (jsonObj.is<const char*>()) return false; // this is defined as a comment
        if (jsonObj.containsKey(HAL_JSON_KEYNAME_TYPE) == false) return false;
        if (jsonObj[HAL_JSON_KEYNAME_TYPE].is<const char*>() == false) return false;
        if (jsonObj[HAL_JSON_KEYNAME_TYPE].as<const char*>() == nullptr) return false;
        if (jsonObj.containsKey(HAL_JSON_KEYNAME_UID) == false) return false;
        if (jsonObj[HAL_JSON_KEYNAME_UID].is<const char*>() == false) return false;
        if (jsonObj[HAL_JSON_KEYNAME_UID].as<const char*>() == nullptr) return false;

        const char* type = jsonObj[HAL_JSON_KEYNAME_TYPE].as<const char*>();
        for (int i=0;DeviceRegistry[i].type != nullptr;i++) {
            if (strcmp(type, DeviceRegistry[i].type) == 0) {
                if (DeviceRegistry[i].Verify_JSON_Function == nullptr) return false;
                if (DeviceRegistry[i].Create_Function == nullptr) return false; // skip devices that dont have this defined

                auto err = DeviceRegistry[i].Verify_JSON_Function(jsonObj);
                if (err != HAL_JSON_VERIFY_JSON_RETURN_OK) {
                    Serial.println(err); // just print to the serial for now
                    return false;
                }
                return true;
            }
        }
        return false;
    }

    void ParseJSON(JsonArray &jsonArray) {
        uint32_t deviceCount = 0;
        uint32_t arraySize = jsonArray.size();

        // First pass: count valid entries
        for (int i=0;i<arraySize;i++) {
            JsonVariant jsonItem = jsonArray[i];
            if (VerifyDeviceJson(jsonItem) == false) continue;
            deviceCount++;
        }
        
        
        // cleanup of prev device list if existent
        if (devices != nullptr) {
            for (int i=0;i<deviceCount;i++) {
                if (devices[i] != nullptr) {
                    delete devices[i];
                    devices[i] = nullptr;
                }
            }
            delete[] devices;
        }
        HAL_JSON::deviceCount = deviceCount;
        // Allocate space for all devices
        devices = new Device*[HAL_JSON::deviceCount];
        
        // Second pass: actually create and store devices
        uint32_t index = 0;
        for (int i=0;i<arraySize;i++) {
            JsonVariant jsonItem = jsonArray[i];
            if (VerifyDeviceJson(jsonItem) == false) continue;
            devices[index] = CreateDeviceFromJSON(jsonItem);
            index++;
        }
    }
}