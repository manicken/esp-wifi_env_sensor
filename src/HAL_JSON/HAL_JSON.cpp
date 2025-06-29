
#include "HAL_JSON.h"

namespace HAL_JSON {

    Device** Manager::devices = nullptr;
    uint32_t Manager::deviceCount = 0;

    Device* Manager::CreateDeviceFromJSON(const JsonVariant &jsonObj) {
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
    bool Manager::VerifyDeviceJson(const JsonVariant &jsonObj) {
        
        if (!ValidateJsonStringField(jsonObj, HAL_JSON_KEYNAME_TYPE)) return false;
        

        const char* type = jsonObj[HAL_JSON_KEYNAME_TYPE].as<const char*>();
        for (int i=0;DeviceRegistry[i].type != nullptr;i++) {
            if (strcmp(type, DeviceRegistry[i].type) == 0) {
                if (DeviceRegistry[i].useRootUID == UseRootUID::Mandatory)
                    if (!ValidateJsonStringField(jsonObj, HAL_JSON_KEYNAME_UID)) return false;

                if (DeviceRegistry[i].Verify_JSON_Function == nullptr){ GlobalLogger.Error(F("Verify_JSON_Function missing for:"),type); return false; }
                if (DeviceRegistry[i].Create_Function == nullptr){ GlobalLogger.Error(F("Create_Function missing for:"), type); return false; } // skip devices that dont have this defined

                return DeviceRegistry[i].Verify_JSON_Function(jsonObj);
            }
        }
        GlobalLogger.Error(F("VerifyDeviceJson - could not find type:"),type);
        return false;
    }

    bool Manager::ParseJSON(const JsonArray &jsonArray) {
        uint32_t deviceCount = 0;
        uint32_t arraySize = jsonArray.size();
        bool* validDevices = new bool[arraySize]; // dont' forget the delete[] call at end of function
        GPIO_manager::ClearAllReservations(); // when devices are verified they also reservate the pins to include checks for duplicate use
        // First pass: count valid entries
        for (int i=0;i<arraySize;i++) {
            JsonVariant jsonItem = jsonArray[i];
            if (jsonItem.is<const char*>()) continue; // this is defined as a comment
            bool valid = VerifyDeviceJson(jsonItem);
            validDevices[i] = valid;
            if (valid == false) HAL_JSON_VALIDATE_IN_LOOP_FAIL_OPERATION;
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
        String devCountStr = String(deviceCount);
        GlobalLogger.Info(F("Created %u devices\n"), devCountStr.c_str());
        delete[] validDevices; // free memory
        return true;
    }

    Device* Manager::findDevice(UIDPath& path) {
        if (path.empty()) return nullptr;

        if (!devices || deviceCount == 0) return nullptr;

        uint64_t rootUID = path.resetAndGetFirst();

        for (int i=0;i<deviceCount;i++) {
            Device* device = devices[i];
            if (device == nullptr) continue;
#if defined(HAL_JSON_USE_EFFICIENT_FIND)
            if (device->uid == rootUID) {
				if ((device->uidMaxLength == 1) || (path.count() == 1))
					return device;
				else
				{
					Device* dev = device->findDevice(path);
					if (dev != nullptr) return dev;
                    rootUID = path.resetAndGetFirst();
				}
					
			}
            else if (device->uid == 0) { // this will only happen on devices where uidMaxLenght>1
				Device* dev = device->findDevice(path);
				if (dev != nullptr) return dev;
                rootUID = path.resetAndGetFirst();
			}
            
#else
            Device* dev = devices[i]->findDevice(path);
            if (dev != nullptr) return dev;
#endif
        }
        return nullptr;
    }

    bool Manager::read(const HALReadRequest &req) {
        Device* device = findDevice(req.path);
        if (device == nullptr) return false;
        return device->read(req.out_value);
    }
    bool Manager::write(const HALWriteRequest &req) {
        Device* device = findDevice(req.path);
        if (device == nullptr) return false;
        return device->write(req.value);
    }
    bool Manager::read(const HALReadStringRequest &req) {
        Device* device = findDevice(req.path);
        if (device == nullptr) return false;
        return device->read(req.value);
    }
    bool Manager::write(const HALWriteStringRequest &req) {
        Device* device = findDevice(req.path);
        if (device == nullptr) return false;
        return device->write(req.value);
    }

    bool Manager::ReadJSON(const char* path) {
        
        if (LittleFS.exists(path) == false) {
            GlobalLogger.Error(F("ReadJSON - cfg file did not exist"),path);
            return false;
        }
        char* jsonBuffer = nullptr;
        size_t fileSize;
        //int size = LittleFS_ext::getFileSize(path);
        //char* jsonBuffer = new char[size + 1]; // +1 for null char
        if (LittleFS_ext::load_from_file(path, &jsonBuffer, &fileSize) == false)
        {
            GlobalLogger.Error(F("ReadJSON - error could not load json file"),path);
            return false;
        }
        DynamicJsonDocument jsonDoc(fileSize*2);
        DeserializationError error = deserializeJson(jsonDoc, jsonBuffer);
        if (error)
        {
            delete[] jsonBuffer;
            GlobalLogger.Error(F("ReadJSON - deserialization failed: "), error.c_str());
            return false;
        }
        String memUsage = String(jsonDoc.memoryUsage()) + " of " + String(jsonDoc.capacity());
        GlobalLogger.Info(F("jsonDoc.memoryUsage="), memUsage.c_str());
        if (!jsonDoc.is<JsonArray>())
        {
            delete[] jsonBuffer;
            GlobalLogger.Error(F("jsonDoc root is not a JsonArray"));
            return false;
        }
        JsonArray jsonItems = jsonDoc.as<JsonArray>();
        if (jsonItems == nullptr)
        {
            delete[] jsonBuffer;
            GlobalLogger.Error(F("jsonDoc root could not convert to a JsonArray"));
            return false;
        }
        bool parseOk = ParseJSON(jsonItems);
        delete[] jsonBuffer;
        return parseOk;
    }
    void Manager::loop() {
        if (deviceCount == 0) return;
        if (devices == nullptr) return;
        for (int i=0;i<deviceCount;i++) {
            if (devices[i] == nullptr) continue;
            devices[i]->loop();
        }
    }

    void Manager::TEST() {
        String result;
        String cmd = "getDevices";

        HALReadStringRequestValue strVal = {cmd, result};
        UIDPath path("1WTG");
        HALReadStringRequest req{path, strVal};
        if (read(req)) {

            Serial.println(result);
        }

        HALValue value;
        UIDPath path2("1WTG:D2");
        HALReadRequest req2(path2, value);
        if (read(req2)) {
            Serial.println(value.asFloat());
        }

        //if (dispatchWrite())
    }
}