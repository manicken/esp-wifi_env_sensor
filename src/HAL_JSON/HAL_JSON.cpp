
#include "HAL_JSON.h"

namespace HAL_JSON {

    Device** Manager::devices = nullptr;
    int Manager::deviceCount = 0;
    AsyncWebServer *asyncWebserver = nullptr;

    void Manager::setup() {
        
        asyncWebserver = new AsyncWebServer(HAL_JSON_REST_API_PORT);
        asyncWebserver->on(HAL_JSON_REST_API_WRITE_URL "*", HTTP_ANY, restAPI_handleWriteOrRead);
        asyncWebserver->on(HAL_JSON_REST_API_READ_URL "*", HTTP_ANY, restAPI_handleWriteOrRead);

        asyncWebserver->on(HAL_JSON_URL_RELOAD_JSON, HTTP_ANY, [](AsyncWebServerRequest *request){
            if (ReadJSON(String(HAL_JSON_CONFIG_JSON_FILE).c_str()) == false) {
                //AsyncResponseStream *response = request->beginResponseStream("application/json");
                //PrintStreamAdapter adapter(*response);
                //webserver->sendHeader("Content-Type", "application/json");
                //webserver->send(200, "application/json", "");
                GlobalLogger.printAllLogs(Serial, false); // TODO make this print back to request client

                const LogEntry& lastEntry = GlobalLogger.getLastEntry();
                request->send(200, "application/json", "{\"error\":\""+lastEntry.MessageToString()+"\"}");
                
            }
            else
            {
                request->send(200, "application/json", "{\"info\":\"JSON reload ok\"}");
                begin(); // call the begin function on all loaded hal devices
            }
        });
        asyncWebserver->on(HAL_JSON_URL_PRINT_DEVICES, HTTP_ANY, [](AsyncWebServerRequest* request){
            String ret = "{";
            ret += "\"deviceCount\":" + String(deviceCount); 
            ret += ",\"devices\":[";
            for (int i=0;i<deviceCount;i++) {
                ret += "{"+devices[i]->ToString()+"}";
                if (i<deviceCount-1) ret += ",";
            }
            ret += "]}";
            request->send(200, "application/json", ret);
        });

        asyncWebserver->on("/", HTTP_ANY, [](AsyncWebServerRequest *request){
            String message = "HAL JSON Manager - REST API<br>";
            message += "<a href=\"/<cmd>/<type>/<uid>/<optional_value>\">Device command</a><br>";
            request->send(200, "text/html", message);
        });
        asyncWebserver->onNotFound([](AsyncWebServerRequest *request){
            String message = "HAL JSON Manager - REST API<br>";
            message += "invalid url:" + request->url() + "<br>";
            request->send(404, "text/html", message);
        });
        asyncWebserver->begin();
       
        if (ReadJSON(String(HAL_JSON_CONFIG_JSON_FILE).c_str()) == false) {
            Serial.println("error happend while reading and parsing config JSON");
            GlobalLogger.printAllLogs(Serial, false);
        }
        else {
            begin(); // call the begin function on all loaded hal devices
        }
    }

    void Manager::restAPI_handleWriteOrRead(AsyncWebServerRequest *request) { // BIG TODO: refactor this function to handle error cases better
        // Example URL: /write/pwm/tempSensor1/255

        String url = request->url(); // e.g., "/write/pwm/tempSensor1/255"

        // Remove the leading '/'
        if (url.startsWith("/")) url = url.substring(1);  // -> "write/pwm/tempSensor1/255"

        // Split into parts
        int p1 = url.indexOf('/');
        int p2 = url.indexOf('/', p1 + 1);
        int p3 = url.indexOf('/', p2 + 1);

        String command = url.substring(0, p1);
        String type = url.substring(p1 + 1, p2);
        String uid  = url.substring(p2 + 1, p3);
        if (p2 != -1 && p3 != -1) {
            uid = url.substring(p2 + 1, p3);
        }
        String value = "";
        if (p3 != -1) value = url.substring(p3 + 1);

        String message = "";
//#define REST_API_DEBUG_REQ
#ifdef REST_API_DEBUG_REQ
        message += "\"debug\":{";
        message += "\"Command\":\"" + command + "\",";
        message += "\"Type\":\"" + type + "\",";
        message += "\"UID\":\"" + uid + "\",";
        message += "\"Value\":\"" + value + "\"},";
#endif
        //  ██     ██ ██████  ██ ████████ ███████ 
        //  ██     ██ ██   ██ ██    ██    ██      
        //  ██  █  ██ ██████  ██    ██    █████   
        //  ██ ███ ██ ██   ██ ██    ██    ██      
        //   ███ ███  ██   ██ ██    ██    ███████ 
        //                                        
        if (command == HAL_JSON_REST_API_WRITE_CMD)
        {
            if (value.length() > 0 || p3 != -1) {
                if (type == HAL_JSON_REST_API_BOOL_TYPE) {
                    uint32_t uintValue = 0;
                    if (value == "true" || value == "1") {
                        uintValue = 1;
                    } else if (value == "false" || value == "0") {
                        uintValue = 0;
                    } else {
                        message += "{\"error\":\"Invalid boolean value.\"}";
                        request->send(200, "application/json", message);
                        return;
                    }
                    UIDPath uidPath(uid.c_str());
                    HALValue halValue = uintValue;
                    HALWriteRequest req(uidPath, halValue);
                    //uint32_t uidInt = (uint32_t) strtoul(uid.c_str(), nullptr, 16);
                    if (write(req)) {
                        message += "\"info\":{\"Value written\":\"";
                        message += uintValue;
                        message += "\"}";
                    }
                    else {
                        const LogEntry& lastEntry = GlobalLogger.getLastEntry();
                        message += "\"error\":\"";
                        message += lastEntry.MessageToString();
                        message += "\"";
                    }
                }
                else if (type == HAL_JSON_REST_API_UINT32_TYPE) {
                    // Convert value to integer
                    uint32_t uintValue = (uint32_t) strtoul(value.c_str(), nullptr, 10);
                    Serial.print("devmgr write uint32 value:");
                    Serial.println(uintValue);
                    //uint32_t uidInt = (uint32_t) strtoul(uid.c_str(), nullptr, 16);
                    UIDPath uidPath(uid.c_str());
                    HALValue halValue = uintValue;
                    HALWriteRequest req(uidPath, halValue);
                    if (write(req)) {
                        message += "\"info\":{\"Value written\":\"";
                        message += uintValue;
                        message += "\"}";
                    }
                    else {
                        const LogEntry& lastEntry = GlobalLogger.getLastEntry();
                        message += "\"error\":\"";
                        message += lastEntry.MessageToString();
                        message += "\"";
                    }

                } else if (type == HAL_JSON_REST_API_STRING_TYPE) {
                    UIDPath uidPath(uid.c_str());
                    String result;
                    HALWriteStringRequestValue strHalValue(value, result);
                    
                    HALWriteStringRequest req(uidPath, strHalValue);
                    if (write(req)) {
                        message += "\"info\":{\"String written\":\"";
                        message += value;
                        message += "\"}";
                    }
                    else {
                        const LogEntry& lastEntry = GlobalLogger.getLastEntry();
                        message += "\"error\":\"";
                        message += lastEntry.MessageToString();
                        message += "\"";
                    }

                } else if (type == HAL_JSON_REST_API_JSON_STR_TYPE) {
                    UIDPath uidPath(uid.c_str());
                    String result;
                    HALWriteStringRequestValue strHalValue(value, result);
                    
                    HALWriteStringRequest req(uidPath, strHalValue);
                    if (write(req)) {
                        message += "\"info\":{\"Json written\":";
                        message += value;
                        message += "}";
                    }
                    else {
                        const LogEntry& lastEntry = GlobalLogger.getLastEntry();
                        message += "\"error\":\"";
                        message += lastEntry.MessageToString();
                        message += "\"";
                    }
                }
                else {
                    message += "\"error\":\"Unknown type for writing.\"";
                }
            } else {
                message += "\"error\":\"No value provided for writing.\"";
            }
        }
        //  ██████  ███████  █████  ██████  
        //  ██   ██ ██      ██   ██ ██   ██ 
        //  ██████  █████   ███████ ██   ██ 
        //  ██   ██ ██      ██   ██ ██   ██ 
        //  ██   ██ ███████ ██   ██ ██████  
        else if (command == HAL_JSON_REST_API_READ_CMD)
        {
            if (type == HAL_JSON_REST_API_BOOL_TYPE) {
                UIDPath uidPath(uid.c_str());
                HALValue halValue;
                HALReadRequest req(uidPath, halValue);

                if (read(req)) {
                    message += DeviceConstStrings::value;//"\"value\":\"";
                    message += halValue.asUInt();
                    //message += + "\"";
                } else {
                    const LogEntry& lastEntry = GlobalLogger.getLastEntry();
                    message += "\"error\":\"";
                    message += lastEntry.MessageToString();
                    message += "\"";
                }
            } else if (type == HAL_JSON_REST_API_UINT32_TYPE) {
                UIDPath uidPath(uid.c_str());
                HALValue halValue;
                HALReadRequest req(uidPath, halValue);
                if (read(req)) {
                    message += DeviceConstStrings::value;//"\"value\":\"";
                    message += halValue.asUInt();
                    //message += "\"";
                } else {
                    const LogEntry& lastEntry = GlobalLogger.getLastEntry();
                    message += "\"error\":\"";
                    message += lastEntry.MessageToString();
                    message += "\"";
                }
            } else if (type == HAL_JSON_REST_API_FLOAT_TYPE) {
                UIDPath uidPath(uid.c_str());
                if (value.length() == 0) {
                    HALValue halValue;
                    HALReadRequest req(uidPath, halValue);
                
                    if (read(req)) {
                        message += DeviceConstStrings::value;//"\"value\":\"";
                        message += halValue.asFloat();
                        //message += "\"";
                    } else {
                        const LogEntry& lastEntry = GlobalLogger.getLastEntry();
                        message += "\"error\":\"";
                        message += lastEntry.MessageToString();
                        message += "\"";
                    }
                }
                else {
                    HALValue halValue;
                    HALReadValueByCmd valByCmd(halValue, value); // value here is cmd
                    HALReadValueByCmdReq req(uidPath, valByCmd);
                
                    if (read(req)) {
                        message += DeviceConstStrings::value;//"\"value\":\"";
                        message += halValue.asFloat();
                        //message += "\"";
                    } else {
                        const LogEntry& lastEntry = GlobalLogger.getLastEntry();
                        message += "\"error\":\"";
                        message += lastEntry.MessageToString();
                        message += "\"";
                    }
                }
            } else if (type == HAL_JSON_REST_API_STRING_TYPE) {
                UIDPath uidPath(uid.c_str());
                String result;
                HALReadStringRequestValue strHalValue(value, result);
                
                HALReadStringRequest req(uidPath, strHalValue);
                if (read(req)) {
                    message += DeviceConstStrings::value;//"\"value\":";
                    message += "\"";
                    message += result;
                    message += "\"";
                }
                else {
                    const LogEntry& lastEntry = GlobalLogger.getLastEntry();
                    message += "\"error\":\"";
                    message += lastEntry.MessageToString();
                    message += "\"";
                }
            } else {
                message += "\"error\":\"Unknown type for reading.\"";
            }
        } else {
            message += "\"error\":\"Unknown command.\"";
        }
        message = "{" + message;
        message += "}";
        request->send(200, "application/json", message);
    }

    void Manager::reloadJSON() {
        
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
        
        if (!ValidateJsonStringField(jsonObj, HAL_JSON_KEYNAME_TYPE)) return false;
        

        const char* type = jsonObj[HAL_JSON_KEYNAME_TYPE].as<const char*>();
        for (int i=0;DeviceRegistry[i].typeName != nullptr;i++) {
            if (strcmp(type, DeviceRegistry[i].typeName) == 0) {
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
        //Serial.println("PArse json thianasoidnoasidnasoidnsaiodnsaodinasdoiandoisandiosndoiasnd");
        uint32_t deviceCount = 0;
        size_t arraySize = jsonArray.size();
        bool* validDevices = new bool[arraySize]; // dont' forget the delete[] call at end of function
        GPIO_manager::ClearAllReservations(); // when devices are verified they also reservate the pins to include checks for duplicate use
        // First pass: count valid entries
        for (size_t i=0;i<arraySize;i++) {
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
        for (size_t i=0;i<arraySize;i++) {
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
                //Serial.println(F("device->uid == rootUID"));
				if ((device->uidMaxLength == 1) || (path.count() == 1))
					return device;
				else
				{
					/*Device* dev = device->findDevice(path);
					if (dev != nullptr) return dev;
                    rootUID = path.resetAndGetFirst();*/
                    // if here then the device will not be found as then
                    // here the root uid have a match then there is no more devices to match
                    // so the following should then be used
                    return device->findDevice(path);
				}
					
			}
            else if (device->uid == 0) { // this will only happen on devices where uidMaxLenght>1
                //Serial.println(F("device->uid == 0"));
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
        if (device == nullptr) { GlobalLogger.Error(F("could not find device: "),req.path.ToString().c_str()); return false; }
        //Serial.println(F("found device"));
        return device->read(req.out_value);
    }
    bool Manager::write(const HALWriteRequest &req) {
        Device* device = findDevice(req.path);
        if (device == nullptr) { GlobalLogger.Error(F("could not find device: "),req.path.ToString().c_str()); return false; }
        //Serial.println(F("found device"));
        return device->write(req.value);
    }
    bool Manager::read(const HALReadStringRequest &req) {
        Device* device = findDevice(req.path);
        if (device == nullptr) { GlobalLogger.Error(F("could not find device: "),req.path.ToString().c_str()); return false; }
        //Serial.println(F("found device"));
        return device->read(req.value);
    }
    bool Manager::write(const HALWriteStringRequest &req) {
        Device* device = findDevice(req.path);
        if (device == nullptr) { GlobalLogger.Error(F("could not find device: "),req.path.ToString().c_str()); return false; }
        //Serial.println(F("found device"));
        return device->write(req.value);
    }
    bool Manager::read(const HALReadValueByCmdReq &req) {
        Device* device = findDevice(req.path);
        if (device == nullptr) { GlobalLogger.Error(F("could not find device: "),req.path.ToString().c_str()); return false; }
        //Serial.println(F("found device"));
        return device->read(req.valByCmd);
    }
    bool Manager::write(const HALWriteValueByCmdReq &req) {
        Device* device = findDevice(req.path);
        if (device == nullptr) { GlobalLogger.Error(F("could not find device: "),req.path.ToString().c_str()); return false; }
        //Serial.println(F("found device"));
        return device->write(req.valByCmd);
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
        size_t jsonDocBufferSize = (size_t)((float)fileSize * 1.5f);
        DynamicJsonDocument jsonDoc(jsonDocBufferSize);
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
        const JsonArray& jsonItems = jsonDoc.as<JsonArray>();
        
        bool parseOk = ParseJSON(jsonItems);
        delete[] jsonBuffer;
        if (parseOk == false) Serial.println("ParseJSON(jsonItems) fail");
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