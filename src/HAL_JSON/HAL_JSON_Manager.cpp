
#include "HAL_JSON_Manager.h"

namespace HAL_JSON {

    Device** Manager::devices = nullptr;
    int Manager::deviceCount = 0;
    int Manager::reloadVersion = 0;
    
    int Manager::DeviceCount() {
        return deviceCount;
    }

    int* Manager::ReloadVersionPtr() {
        return &reloadVersion;
    }

    void Manager::setup() {
#ifdef _WIN32
        if (ReadJSON(String(HAL_JSON_CONFIG_JSON_FILE).c_str()+1) == false) { // remove the leading /
#else
        if (ReadJSON(String(HAL_JSON_CONFIG_JSON_FILE).c_str()) == false) {
#endif
            Serial.println("error happend while reading and parsing config JSON");
            GlobalLogger.printAllLogs(Serial, false);
        }
        else {
            begin(); // call the begin function on all loaded hal devices
        }
    }

    std::string Manager::ToString() {
        std::string ret;
        ret += "\"deviceCount\":" + std::to_string(deviceCount); 
        ret += ",\"devices\":[";
        for (int i=0;i<deviceCount;i++) {
            ret += "{";
            ret += devices[i]->ToString().c_str();
            ret += "}";
            if (i<deviceCount-1) ret += ",";
        }
        ret += "]";
        return ret;
    }

    Device* Manager::CreateDeviceFromJSON(const JsonVariant &jsonObj) {
        const char* type = jsonObj[HAL_JSON_KEYNAME_TYPE].as<const char*>();
        for (int i=0;DeviceRegistry[i].typeName != nullptr;i++) {
            if (strcmp(type, DeviceRegistry[i].typeName) == 0) {
                if (DeviceRegistry[i].Create_Function == nullptr) {
                    GlobalLogger.Error(F("CreateDeviceFromJSON - Create_Function == nullptr - something is very wrong if this happens"));
                    return nullptr; // should never happen as VerifyJson is called before and do actually verify that this pointer do point to something
                }
                return DeviceRegistry[i].Create_Function(jsonObj, DeviceRegistry[i].typeName);
            }
        }
        // should never happen as VerifyJson is called before and do actually verify that this function should work
        GlobalLogger.Error(F("CreateDeviceFromJSON - something is very wrong if this happens"));
        return nullptr; // no match
    }
    bool Manager::VerifyDeviceJson(const JsonVariant &jsonObj) {
        
        if (!ValidateJsonStringField(jsonObj, HAL_JSON_KEYNAME_TYPE)) { SET_ERR_LOC(HAL_JSON_ERROR_SOURCE_MGR_VERIFY_DEVICE); return false; }

        const char* type = jsonObj[HAL_JSON_KEYNAME_TYPE].as<const char*>();
        for (int i=0;DeviceRegistry[i].typeName != nullptr;i++) {

            if (strcmp(type, DeviceRegistry[i].typeName) == 0) {
                if (DeviceRegistry[i].useRootUID == UseRootUID::Mandatory)
                    if (!ValidateJsonStringField(jsonObj, HAL_JSON_KEYNAME_UID)) { SET_ERR_LOC(HAL_JSON_ERROR_SOURCE_MGR_VERIFY_DEVICE); return false; }

                if (DeviceRegistry[i].Verify_JSON_Function == nullptr){ GlobalLogger.Error(F("Verify_JSON_Function missing for:"),type); return false; }
                if (DeviceRegistry[i].Create_Function == nullptr){ GlobalLogger.Error(F("Create_Function missing for:"), type); return false; } // skip devices that dont have this defined

                return DeviceRegistry[i].Verify_JSON_Function(jsonObj);
            }
        }
     
        GlobalLogger.Error(F("VerifyDeviceJson - could not find type:"),type);
        return false;
    }

    bool Manager::ParseJSON(const JsonArray &jsonArray) {
        //Serial.println("PArse json thianasoidnoasidnasoidnsaiodnsaodinasdoiandoisandiosndoiasnd");
        uint32_t deviceCount = 0;
        int arraySize = jsonArray.size();
        bool* validDevices = new bool[arraySize]; // dont' forget the delete[] call at end of function
        GPIO_manager::ClearAllReservations(); // when devices are verified they also reservate the pins to include checks for duplicate use

        // First pass: count valid entries
        for (int i=0;i<arraySize;i++) {

            JsonVariant jsonItem = jsonArray[i];

            if (IsConstChar(jsonItem) == true) { validDevices[i] = false;  continue; } // comment item

            if (Device::DisabledInJson(jsonItem) == true) { validDevices[i] = false;  continue; } // disabled

            bool valid = VerifyDeviceJson(jsonItem);

            validDevices[i] = valid;
            if (valid == false) HAL_JSON_VALIDATE_IN_LOOP_FAIL_OPERATION; // could either be continue; or return false depending if strict mode is on/off
            deviceCount++;
        }

        // cleanup of prev device list if existent
        if (devices != nullptr) {
            for (int i=0;i<HAL_JSON::Manager::deviceCount;i++) {
                if (devices[i] != nullptr) {
                    delete devices[i];
                    devices[i] = nullptr;
                }
            }
            delete[] devices;
            devices = nullptr;
        }
        HAL_JSON::Manager::deviceCount = deviceCount;
        if (deviceCount == 0) {
            GlobalLogger.Error(F("The loaded JSON cfg does not contain any valid devices!\n" 
                                 "Hint: Check that all entries have 'type' and 'uid' fields, and match known types."));
            return false;
        }

        // Allocate space for all devices
        devices = new Device*[HAL_JSON::Manager::deviceCount]();

        if (devices == nullptr) {
            GlobalLogger.Error(F("Failed to allocate device array"));
            return false;
        }

        GPIO_manager::ClearAllReservations(); 
        // Second pass: actually create and store devices
        uint32_t index = 0;
        for (int i=0;i<arraySize;i++) {
            JsonVariant jsonItem = jsonArray[i];
            //if (VerifyDeviceJson(jsonItem) == false) continue; // ************************************************************ now as we dont run this again the pins are not allocated anymore but we don't really need to take care of that as it's part of the validate device check anyway
            if (validDevices[i] == false) continue;
            devices[index++] = CreateDeviceFromJSON(jsonItem);
        }
        std::string devCountStr = std::to_string(deviceCount);
        GlobalLogger.Info(F("Created devices: "), devCountStr.c_str());
        delete[] validDevices; // free memory
        return true;
    }

    Device* Manager::findDevice(UIDPath& path) {
        if (path.empty()) {
            GlobalLogger.Error(F("findDevice failed: empty path"));
            return nullptr;
        }
        if (!devices || deviceCount == 0) {
            GlobalLogger.Error(F("findDevice failed: no devices"));
            return nullptr;
        }

        HAL_UID rootUID = path.resetAndGetFirst();

        Device* indirectMatch = nullptr;

        for (int i=0;i<deviceCount;i++) {
            Device* device = devices[i];
            if (device == nullptr) continue;

            if (device->uid == rootUID) {
                //Serial.println(F("device->uid == rootUID"));
				if ((device->uidMaxLength == 1) || (path.count() == 1))
					return device; // direct match allways return valid device
				else
				{
					// If a device matched the rootUID but couldn't directly resolve the full path,
                    // attempt an indirect lookup via the matched device.
                    indirectMatch = device->findDevice(path); // indirect match can return nullptr
                    break; // No need to continue — rootUID match is unique
				}
			}
            else if (device->uid == 0) { // this will only happen on devices where uidMaxLenght>1
                //Serial.println(F("device->uid == 0"));
				Device* dev = device->findDevice(path);
				if (dev != nullptr) return dev; // match allways return valid device
                rootUID = path.resetAndGetFirst();
			}
        }
        if (indirectMatch != nullptr) {
            return indirectMatch;
        }
        GlobalLogger.Error(F("could not find device: "),path.ToString().c_str());
        return nullptr;
    }

    HALOperationResult  Manager::read(const HALReadRequest &req) {
        Device* device = findDevice(req.path);
        if (device == nullptr) { return HALOperationResult::DeviceNotFound; }
        //Serial.println(F("found device"));
        return device->read(req.out_value);
    }
    HALOperationResult  Manager::write(const HALWriteRequest &req) {
        Device* device = findDevice(req.path);
        if (device == nullptr) { return HALOperationResult::DeviceNotFound; }
        //Serial.println(F("found device"));
        return device->write(req.value);
    }
    HALOperationResult  Manager::read(const HALReadStringRequest &req) {
        Device* device = findDevice(req.path);
        if (device == nullptr) { return HALOperationResult::DeviceNotFound; }
        //Serial.println(F("found device"));
        return device->read(req.value);
    }
    HALOperationResult  Manager::write(const HALWriteStringRequest &req) {
        Device* device = findDevice(req.path);
        if (device == nullptr) { return HALOperationResult::DeviceNotFound; }
        //Serial.println(F("found device"));
        return device->write(req.value);
    }
    HALOperationResult  Manager::read(const HALReadValueByCmdReq &req) {
        Device* device = findDevice(req.path);
        if (device == nullptr) { return HALOperationResult::DeviceNotFound; }
        //Serial.println(F("found device"));
        return device->read(req.valByCmd);
    }
    HALOperationResult  Manager::write(const HALWriteValueByCmdReq &req) {
        Device* device = findDevice(req.path);
        if (device == nullptr) { return HALOperationResult::DeviceNotFound; }
        //Serial.println(F("found device"));
        return device->write(req.valByCmd);
    }

    bool Manager::ReadJSON(const char* path) {
        if (path == nullptr) {
            GlobalLogger.Error(F("ReadJSON - path cannot be empty "));
            return false;
        }
        if (LittleFS.exists(path) == false) {
            GlobalLogger.Error(F("ReadJSON - cfg file did not exist: "),path);
            return false;
        }

        char* jsonBuffer = nullptr;
        size_t fileSize;

        if (LittleFS_ext::load_from_file(path, &jsonBuffer, &fileSize) == false)
        {
            GlobalLogger.Error(F("ReadJSON - error could not load json file: "),path);
            return false;
        }
#ifdef _WIN32
        size_t jsonDocBufferSize = fileSize * 10; // very safe mem
#else
        size_t jsonDocBufferSize = (size_t)((float)fileSize * 1.5f);
#endif
        DynamicJsonDocument jsonDoc(jsonDocBufferSize);
        DeserializationError error = deserializeJson(jsonDoc, jsonBuffer);
        if (error)
        {
            delete[] jsonBuffer;
            GlobalLogger.Error(F("ReadJSON - deserialization failed: "), error.c_str());
            return false;
        }

        std::string memUsage = std::to_string(jsonDoc.memoryUsage()) + " of " + std::to_string(jsonDoc.capacity());
        GlobalLogger.Info(F("jsonDoc.memoryUsage="), memUsage.c_str());
        if (!jsonDoc.is<JsonArray>())
        {
            delete[] jsonBuffer;
            GlobalLogger.Error(F("jsonDoc root is not a JsonArray"));
            return false;
        }
        const JsonArray& jsonItems = jsonDoc.as<JsonArray>();

        bool parseOk = ParseJSON(jsonItems);

        delete[] jsonBuffer;
        if (parseOk == false) {
            //GlobalLogger.Error(F("ParseJSON(jsonItems) fail"));
            //Serial.println("");
        }
        if (parseOk == true) reloadVersion++;

        return parseOk;
    }
    void Manager::begin() {
        for (int i=0;i<deviceCount;i++) {
            Device* device = devices[i];
            if (device == nullptr) continue;
            device->begin();
        }
    }

    void Manager::loop() {
        if ((devices == nullptr) || (deviceCount == 0)) return;

        for (int i=0;i<deviceCount;i++) {
            Device* device = devices[i];
            if (device == nullptr) continue;
            device->loop();
            if (device->LoopTaskDone()) {
                // do something here
                // here we can have a callback
                // right now the only use for this would be to signal to RuleManager (future implementation, next in queue)
                // so that reads from rules are synced with refresh rates of the different devices
            }
        }
    }
    /** 
     * the following is not intended to be used  
     * it's just to check that everything is correct
     * in the future it could be a real TEST function
    */
    void Manager::TEST() {
        std::string result;
        ZeroCopyString cmd("getDevices");

        HALReadStringRequestValue strVal = {cmd, result};
        UIDPath path("1WTG");
        HALReadStringRequest req{path, strVal};
        if (read(req) == HALOperationResult::Success) {

            Serial.println(result.c_str());
        }

        HALValue value;
        UIDPath path2("1WTG:D2");
        HALReadRequest req2(path2, value);
        if (read(req2) == HALOperationResult::Success) {
            Serial.println(value.asFloat());
        }
    }
}