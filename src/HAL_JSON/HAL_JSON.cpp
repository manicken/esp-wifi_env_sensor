
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
        if (jsonObj.containsKey(HAL_JSON_KEYNAME_TYPE) == false){ Serial.println(HAL_JSON_ERR_MISSING_KEY(HAL_JSON_KEYNAME_TYPE)); return false; }
        if (jsonObj[HAL_JSON_KEYNAME_TYPE].is<const char*>() == false){ Serial.println(HAL_JSON_ERR_VALUE_TYPE(HAL_JSON_KEYNAME_TYPE)); return false; }
        if (jsonObj[HAL_JSON_KEYNAME_TYPE].as<const char*>() == nullptr){ Serial.println(HAL_JSON_ERR_STRING_EMPTY(HAL_JSON_KEYNAME_TYPE)); return false; }
        if (jsonObj.containsKey(HAL_JSON_KEYNAME_UID) == false){ Serial.println(HAL_JSON_ERR_MISSING_KEY(HAL_JSON_KEYNAME_UID)); return false; }
        if (jsonObj[HAL_JSON_KEYNAME_UID].is<const char*>() == false){ Serial.println(HAL_JSON_ERR_VALUE_TYPE(HAL_JSON_KEYNAME_UID)); return false; }
        if (jsonObj[HAL_JSON_KEYNAME_UID].as<const char*>() == nullptr){ Serial.println(HAL_JSON_ERR_STRING_EMPTY(HAL_JSON_KEYNAME_UID)); return false; }

        const char* type = jsonObj[HAL_JSON_KEYNAME_TYPE].as<const char*>();
        for (int i=0;DeviceRegistry[i].type != nullptr;i++) {
            if (strcmp(type, DeviceRegistry[i].type) == 0) {
                if (DeviceRegistry[i].Verify_JSON_Function == nullptr){ Serial.print(F("Verify_JSON_Function missing for:")); Serial.println(type); return false; }
                if (DeviceRegistry[i].Create_Function == nullptr){ Serial.print(F("Create_Function missing for:")); Serial.println(type); return false; } // skip devices that dont have this defined

                auto err = DeviceRegistry[i].Verify_JSON_Function(jsonObj);
                if (err != HAL_JSON_VERIFY_JSON_RETURN_OK) {
                    Serial.println(err); // just print to the serial for now
                    return false;
                }
                return true;
            }
        }

        Serial.print(F("Err - could not find type:"));
        Serial.println(type);
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
            for (int i=0;i<HAL_JSON::deviceCount;i++) {
                if (devices[i] != nullptr) {
                    delete devices[i];
                    devices[i] = nullptr;
                }
            }
            delete[] devices;
            devices = nullptr;
        }
        HAL_JSON::deviceCount = deviceCount;
        if (deviceCount == 0) {
            Serial.println(F("The loaded JSON cfg does not contain any valid devices!"));
            Serial.println(F("Hint: Check that all entries have 'type' and 'uid' fields, and match known types."));
            return;
        }
        // Allocate space for all devices
        devices = new Device*[HAL_JSON::deviceCount]();

        if (devices == nullptr) {
            Serial.println(F("Failed to allocate device array"));
            return;
        }
        
        // Second pass: actually create and store devices
        uint32_t index = 0;
        for (int i=0;i<arraySize;i++) {
            JsonVariant jsonItem = jsonArray[i];
            if (VerifyDeviceJson(jsonItem) == false) continue;
            devices[index] = CreateDeviceFromJSON(jsonItem);
            index++;
        }
        Serial.printf("Created %u devices\n", deviceCount);

    }

    Device* findDevice(uint64_t uid) {
        if (deviceCount == 0) return nullptr;
        if (devices == nullptr) return nullptr;
        for (int i=0;i<deviceCount;i++) {
            if (devices[i] == nullptr) continue;
            if (devices[i]->uid == uid) return devices[i];
        }
        return nullptr;
    }

    template<typename RequestType>
    bool dispatchRead(const RequestType& req) {
        Device* device = findDevice(req.path.root());
        return device ? device->read(req) : false;
    }

    template<typename RequestType>
    bool dispatchWrite(const RequestType& req) {
        Device* device = findDevice(req.path.root());
        return device ? device->write(req) : false;
    }

    bool read(const HALReadRequest &req) {
        Device* device = findDevice(req.path.root());
        if (device == nullptr) return false;
        return device->read(req);
    }
    bool write(const HALWriteRequest &req) {
        Device* device = findDevice(req.path.root());
        if (device == nullptr) return false;
        return device->write(req);
    }
    bool read(const HALReadStringRequest &req) {
        Device* device = findDevice(req.path.root());
        if (device == nullptr) return false;
        return device->read(req);
    }
    bool write(const HALWriteStringRequest &req) {
        Device* device = findDevice(req.path.root());
        if (device == nullptr) return false;
        return device->write(req);
    }
    void loop() {
        if (deviceCount == 0) return;
        if (devices == nullptr) return;
        for (int i=0;i<deviceCount;i++) {
            if (devices[i] == nullptr) continue;
            devices[i]->loop();
        }
    }

    void TEST() {
        String result;
        HALReadStringRequest req{UIDPath("D1","1WTG"), result};
        if (dispatchRead(req)) {
            Serial.println(result);
        }

        HALValue value;
        HALReadRequest req2(UIDPath("D2","1WTG"), value);
        if (dispatchRead(req2)) {
            Serial.println(value.asFloat());
        }

        //if (dispatchWrite())
    }
}