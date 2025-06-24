
#include "HAL_JSON.h"

namespace HAL_JSON {
    Device **devices = nullptr;
    uint32_t deviceCount = 0;

    Device* CreateDeviceFromJSON(const JsonVariant &jsonObj) {
        const char* type = jsonObj[HAL_JSON_KEYNAME_TYPE].as<const char*>();
        for (int i=0;DeviceRegistry[i].type != nullptr;i++) {
            if (strcmp(type, DeviceRegistry[i].type) == 0) {
                if (DeviceRegistry[i].Create_Function == nullptr) {
                    GlobalLogger.Error(F("CreateDeviceFromJSON - Create_Function == nullptr - something is very wrong if this happens"));
                    return nullptr; // should never happen as VerifyJson is called before and do actually verify that this pointer do point to something
                }
                return DeviceRegistry[i].Create_Function(jsonObj);
            }
        }
        // should never happen as VerifyJson is called before and do actually verify that this function should work
        GlobalLogger.Error(F("CreateDeviceFromJSON - something is very wrong if this happens"));
        return nullptr; // no match
    }
    bool VerifyDeviceJson(const JsonVariant &jsonObj) {
        if (jsonObj.is<const char*>()) return false; // this is defined as a comment
        if (jsonObj.containsKey(HAL_JSON_KEYNAME_TYPE) == false){ GlobalLogger.Error(HAL_JSON_ERR_MISSING_KEY(HAL_JSON_KEYNAME_TYPE)); return false; }
        if (jsonObj[HAL_JSON_KEYNAME_TYPE].is<const char*>() == false){ GlobalLogger.Error(HAL_JSON_ERR_VALUE_TYPE(HAL_JSON_KEYNAME_TYPE)); return false; }
        if (jsonObj[HAL_JSON_KEYNAME_TYPE].as<const char*>() == nullptr){ GlobalLogger.Error(HAL_JSON_ERR_STRING_EMPTY(HAL_JSON_KEYNAME_TYPE)); return false; }
        if (jsonObj.containsKey(HAL_JSON_KEYNAME_UID) == false){ GlobalLogger.Error(HAL_JSON_ERR_MISSING_KEY(HAL_JSON_KEYNAME_UID)); return false; }
        if (jsonObj[HAL_JSON_KEYNAME_UID].is<const char*>() == false){ GlobalLogger.Error(HAL_JSON_ERR_VALUE_TYPE(HAL_JSON_KEYNAME_UID)); return false; }
        if (jsonObj[HAL_JSON_KEYNAME_UID].as<const char*>() == nullptr){ GlobalLogger.Error(HAL_JSON_ERR_STRING_EMPTY(HAL_JSON_KEYNAME_UID)); return false; }

        const char* type = jsonObj[HAL_JSON_KEYNAME_TYPE].as<const char*>();
        for (int i=0;DeviceRegistry[i].type != nullptr;i++) {
            if (strcmp(type, DeviceRegistry[i].type) == 0) {
                if (DeviceRegistry[i].Verify_JSON_Function == nullptr){ GlobalLogger.Error(F("Verify_JSON_Function missing for:"),type); return false; }
                if (DeviceRegistry[i].Create_Function == nullptr){ GlobalLogger.Error(F("Create_Function missing for:"), type); return false; } // skip devices that dont have this defined

                return DeviceRegistry[i].Verify_JSON_Function(jsonObj);
            }
        }
        GlobalLogger.Error(F("Err - could not find type:"),type);
        return false;
    }

    bool ParseJSON(const JsonArray &jsonArray) {
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
            GlobalLogger.Error(F("The loaded JSON cfg does not contain any valid devices!\n" 
                                 "Hint: Check that all entries have 'type' and 'uid' fields, and match known types."));
            return false;
        }
        // Allocate space for all devices
        devices = new Device*[HAL_JSON::deviceCount]();

        if (devices == nullptr) {
            GlobalLogger.Error(F("Failed to allocate device array"));
            return false;
        }
        
        // Second pass: actually create and store devices
        uint32_t index = 0;
        for (int i=0;i<arraySize;i++) {
            JsonVariant jsonItem = jsonArray[i];
            if (VerifyDeviceJson(jsonItem) == false) continue;
            devices[index++] = CreateDeviceFromJSON(jsonItem);
        }
        String devCountStr = String(deviceCount);
        GlobalLogger.Info(F("Created %u devices\n"), devCountStr.c_str());
        return true;
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

    bool ReadJSON(const char* path) {
        
        if (LittleFS.exists(path) == false) {
            GlobalLogger.Error(F("ReadJSON - cfg file did not exist"),path);
            return false;
        }
        int size = LittleFS_ext::getFileSize(path);
        char jsonBuffer[size + 1]; // +1 for null char
        if (LittleFS_ext::load_from_file(path, jsonBuffer) == false)
        {
            GlobalLogger.Error(F("ReadJSON - error could not load json file"),path);
            return false;
        }
        DynamicJsonDocument jsonDoc(size*2);
        DeserializationError error = deserializeJson(jsonDoc, jsonBuffer);
        if (error)
        {
            GlobalLogger.Error(F("ReadJSON - deserialization failed: "), error.c_str());
            return false;
        }
        String memUsage = String(jsonDoc.memoryUsage()) + " of " + String(jsonDoc.capacity());
        GlobalLogger.Info(F("jsonDoc.memoryUsage="), memUsage.c_str());
        if (!jsonDoc.is<JsonArray>()) {
            GlobalLogger.Error(F("jsonDoc root is not a JsonArray"));
            return false;
        }
        JsonArray jsonItems = jsonDoc.as<JsonArray>();
        if (jsonItems == nullptr) {
            GlobalLogger.Error(F("jsonDoc root could not convert to a JsonArray"));
            return false;
        }
        return ParseJSON(jsonItems);
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