
#include "HAL_JSON_REST.h"

namespace HAL_JSON {

    AsyncWebServer* REST::asyncWebserver = nullptr;

    void REST::setup() {
        asyncWebserver = new AsyncWebServer(HAL_JSON_REST_API_PORT);
        asyncWebserver->on(HAL_JSON_REST_API_WRITE_URL "*", HTTP_ANY, handleWriteOrRead);
        asyncWebserver->on(HAL_JSON_REST_API_READ_URL "*", HTTP_ANY, handleWriteOrRead);

        asyncWebserver->on(HAL_JSON_URL_RELOAD_JSON, HTTP_ANY, [](AsyncWebServerRequest *request){
            if (Manager::ReadJSON(String(HAL_JSON_CONFIG_JSON_FILE).c_str()) == false) {
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
                Manager::begin(); // call the begin function on all loaded hal devices
            }
        });
        asyncWebserver->on(HAL_JSON_URL_PRINT_DEVICES, HTTP_ANY, [](AsyncWebServerRequest* request){
            request->send(200, "application/json", Manager::ToString().c_str());
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
    }

    void REST::handleWriteOrRead(AsyncWebServerRequest *request) { // BIG TODO: refactor this function to handle error cases better
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
        std::string value = "";
        if (p3 != -1) value = url.substring(p3 + 1).c_str();

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
                    if (Manager::write(req)) {
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
                    if (Manager::write(req)) {
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
                    std::string result;
                    HALWriteStringRequestValue strHalValue(value, result);
                    
                    HALWriteStringRequest req(uidPath, strHalValue);
                    if (Manager::write(req)) {
                        message += "\"info\":{\"String written\":\"";
                        message += value.c_str();
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
                    std::string result;
                    HALWriteStringRequestValue strHalValue(value, result);
                    
                    HALWriteStringRequest req(uidPath, strHalValue);
                    if (Manager::write(req)) {
                        message += "\"info\":{\"Json written\":";
                        message += value.c_str();
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

                if (Manager::read(req)) {
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
                if (Manager::read(req)) {
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
                
                    if (Manager::read(req)) {
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
                
                    if (Manager::read(req)) {
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
                std::string result;
                HALReadStringRequestValue strHalValue(value, result);
                
                HALReadStringRequest req(uidPath, strHalValue);
                if (Manager::read(req)) {
                    message += DeviceConstStrings::value;//"\"value\":";
                    message += "\"";
                    message += result.c_str();
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
}