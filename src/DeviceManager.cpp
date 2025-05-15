#include "DeviceManager.h"
#include <cstdlib> // For std::strtoul
#include "ConvertHelper.h"

namespace DeviceManager
{
    std::string lastError;

    WEBSERVER_TYPE *webserver = nullptr;
    AsyncWebServer *asyncWebserver = nullptr;

    DHTesp dht;
    OneWire oneWire;
    DallasTemperature dTemp(&oneWire);

    /**************** OneWireBus ****************/
    OneWireBus *oneWireBusses = nullptr;
    int oneWireBusCount = 0;

    String OneWireBus::ToString() {
        String str = "";
        str.concat("uid="); str.concat(uid);
        str.concat(", pin="); str.concat(pin);
        return str;
    }
    int getOneWireBusPin(int busUid)
    {
        if (oneWireBusCount == 0) return -1;
        if (oneWireBusses == nullptr) return -1;
        for (int i=0;i<oneWireBusCount;i++)
        {
            if (oneWireBusses[i].uid != busUid) continue;
            return oneWireBusses[i].pin;
        }
        return -1;
    }

    /**************** Device (base class) ****************/
    Device **devices = nullptr;
    uint32_t deviceCount = 0;

    String DeviceTypeToString(DeviceType type)
    {
        if (type == DeviceType::OneWireBus) return "OneWireBus";
        else if (type == DeviceType::OneWireTemp) return "OneWireTemp";
        else if (type == DeviceType::ADC) return "ADC";
        else if (type == DeviceType::DAC) return "DAC";
        else if (type == DeviceType::DHT) return "DHT";
        else if (type == DeviceType::DIN) return "DIN";
        else if (type == DeviceType::DOUT) return "DOUT";
        else if (type == DeviceType::PWM) return "PWM";
        else if (type == DeviceType::TX433) return "TX433";
        else return "Unknown";
    }

    String Device::ToString() {
        String str;
        str.concat("uid="); str.concat(uid);
        str.concat(", type="); str.concat(DeviceTypeToString(type));
        str.concat(", pin="); str.concat(pin);
        return str;
    }

    /**************** OneWireDevice ****************/
    bool OneWireDevice::IsValid() {
        return romid != nullptr; 
    }
    OneWireDevice::~OneWireDevice() {
        if (romid != nullptr)
            delete[] romid;
            romid = nullptr;
    }

    /**************** OneWireTempDevice ****************/
    OneWireTempDevice::OneWireTempDevice(uint32_t _uid, uint8_t _pin, const char* romid_hexstr)
    {
        romid = new uint8_t[8]();
        if (Convert::convertHexToBytes(romid_hexstr, romid, 8) == false) { delete[] romid; romid = nullptr;}
        // note later usage must allways check beforehand if romid is nullptr before use
        type = DeviceType::OneWireTemp;
        uid = _uid;
        pin = _pin;
        value = 0;
    }
    String OneWireTempDevice::ToString() {
        String str = Device::ToString();
        str.concat(", romid="); 
        if (romid != nullptr)
            str.concat(Convert::ByteArrayToString(romid, 8).c_str());
        else
            str.concat("nullptr");
        str.concat(", value="); str.concat(value);
        return str;
    }

    /**************** DHTdevice ****************/
    DHTdevice::DHTdevice(const char* dhtTypeStr, uint32_t _uid, uint8_t _pin) {
        type = DeviceType::DHT;
        if (strncmp(dhtTypeStr, DEVICE_MANAGER_JSON_NAME_TYPE_DHT11, sizeof(DEVICE_MANAGER_JSON_NAME_TYPE_DHT11)-1) == 0)
            dhtType = DHT_Type::DHT11;
        else if (strncmp(dhtTypeStr, DEVICE_MANAGER_JSON_NAME_TYPE_DHT22, sizeof(DEVICE_MANAGER_JSON_NAME_TYPE_DHT22)-1) == 0)
            dhtType = DHT_Type::DHT22;
        else if (strncmp(dhtTypeStr, DEVICE_MANAGER_JSON_NAME_TYPE_DHT_AM2302, sizeof(DEVICE_MANAGER_JSON_NAME_TYPE_DHT_AM2302)-1) == 0)
            dhtType = DHT_Type::AM2302;
        else if (strncmp(dhtTypeStr, DEVICE_MANAGER_JSON_NAME_TYPE_DHT_RHT03, sizeof(DEVICE_MANAGER_JSON_NAME_TYPE_DHT_RHT03)-1) == 0)
            dhtType = DHT_Type::RHT03;
        else
            dhtType = DHT_Type::Unknown;

        uid = _uid;
        pin = _pin;
        value = 0;
    }
    String DHTtypeToString(DHT_Type type)
    {
        if (type == DHT_Type::DHT11) return "DHT11";
        else if (type == DHT_Type::DHT22) return "DHT22";
        else if (type == DHT_Type::AM2302) return "AM2302";
        else if (type == DHT_Type::RHT03) return "RHT03";
        else return "unknown";
    }
    String DHTdevice::ToString()
    {
        String str = Device::ToString();
        str.concat(", dhtType="); str.concat(DHTtypeToString(dhtType));
        str.concat(", value="); str.concat(value);
        return str;
    }

    /**************** GPIOdevices ****************/
    DOUTdevice::DOUTdevice(uint32_t _uid, uint8_t _pin)
    {
        type = DeviceType::DOUT;
        uid = _uid;
        pin = _pin;
    }
    DINdevice::DINdevice(uint32_t _uid, uint8_t _pin)
    {
        type = DeviceType::DIN;
        uid = _uid;
        pin = _pin;
    }


    /**************** PWMdevice ****************/
    PWMdevice::PWMdevice(uint32_t _uid, uint8_t _pin, float _frequency, uint8_t _bits, uint8_t _invOut)
    {
        type = DeviceType::PWM;
        uid = _uid;
        pin = _pin;
        frequency = _frequency;
        bits = _bits;
        invOut = _invOut;
    }
    String PWMdevice::ToString()
    {
        String str = Device::ToString();
        str.concat(", frequency="); str.concat(frequency);
        str.concat(", bits="); str.concat(bits);
        str.concat(", invOut="); str.concat(invOut);
        return str;
    }

    /**************** TX433device ****************/
    TX433device::TX433device(uint32_t _uid, uint8_t _pin)
    {
        type = DeviceType::TX433;
        uid = _uid;
        pin = _pin;
    }

    /**************************************************************/
    /********************** JSON helpers **************************/
    /**************************************************************/
    bool isValid_JsonOneWireBus_Item(JsonVariant jsonItem)
    {
        // TODO use lastError
        if (jsonItem == nullptr) { return false; } // failsafe 
        if (jsonItem.is<JsonObject>() == false) { return false; } // failsafe
        if (jsonItem.containsKey(DEVICE_MANAGER_JSON_NAME_PIN) == false) { return false; }
        if (jsonItem[DEVICE_MANAGER_JSON_NAME_PIN].is<int>() == false) { return false; }
        return true;
    }
    bool isValid_JsonDevice_Item(JsonVariant jsonItem, const char*& type, uint32_t *uid) {
        // TODO use lastError
        if (jsonItem == nullptr) { lastError += "@isValid_JsonDevice_Item: jsonItem == nullptr"; return false; }
        if (!jsonItem.is<JsonObject>()) { return false; }
        if (!jsonItem.containsKey(DEVICE_MANAGER_JSON_NAME_TYPE)) { return false; }
        if (!jsonItem[DEVICE_MANAGER_JSON_NAME_TYPE].is<const char*>()) { return false; }
        if (jsonItem.containsKey(DEVICE_MANAGER_JSON_NAME_UID) == false) { return false; }
        // allow uid to be both a integer-number or as a string interprented as a "8 nibble ascii hex number"(32bit value)
        if (jsonItem[DEVICE_MANAGER_JSON_NAME_UID].is<int>()) { 
            if (uid!=nullptr) *uid = jsonItem[DEVICE_MANAGER_JSON_NAME_UID].as<int>();
        }
        else if (jsonItem[DEVICE_MANAGER_JSON_NAME_UID].is<const char*>()){
            const char* uid_asciiHexString = jsonItem[DEVICE_MANAGER_JSON_NAME_UID].as<const char*>();
            if (uid_asciiHexString == nullptr) { return false; }
            char* endptr = nullptr;
            if (uid!=nullptr) { 
                *uid = static_cast<uint32_t>(std::strtoul(uid_asciiHexString, &endptr, 16));
                if (endptr == uid_asciiHexString) {
                    lastError += "Error @ isValid_JsonDevice_Item: No valid conversion could be performed (invalid input).\n";
                } else if (*endptr != '\0') {
                    lastError += "Warning @ isValid_JsonDevice_Item: Additional characters after number: ";
                    lastError += std::string(endptr);
                    lastError += "\n";
                }
            }

        }
        else {
            if (uid!=nullptr) *uid = 0;
            lastError += "@isValid_JsonDevice_Item: uid is either a number or a valid ascii hex number\n";
            return false;
        }
        
        type = jsonItem[DEVICE_MANAGER_JSON_NAME_TYPE].as<const char*>();
        return type != nullptr;
    }
    bool isValid_JsonDHT_Item(JsonVariant jsonItem, const char*& dhtType)
    {
        // TODO use lastError
        if (jsonItem == nullptr) { return false; } // failsafe
        if (jsonItem.is<JsonObject>() == false) { return false; } // failsafe
        if (jsonItem.containsKey(DEVICE_MANAGER_JSON_NAME_PIN) == false) { return false; }
        if (jsonItem[DEVICE_MANAGER_JSON_NAME_PIN].is<int>() == false) { return false; }
        if (jsonItem.containsKey(DEVICE_MANAGER_JSON_NAME_DHT_TYPE) == false) { return false; }

        dhtType = jsonItem[DEVICE_MANAGER_JSON_NAME_DHT_TYPE].as<const char*>();
        return dhtType != nullptr;
    }
    bool isValid_JsonOneWireTemp_Item(JsonVariant jsonItem, const char*& romid, uint32_t *bus)
    {
        // TODO use lastError
        if (jsonItem == nullptr) { lastError += "isValid_JsonOneWireTemp_Item: jsonItem == nullptr\n"; return false; } // failsafe
        if (jsonItem.is<JsonObject>() == false) { lastError += "isValid_JsonOneWireTemp_Item: jsonItem.is<JsonObject>() == false\n"; return false; } // failsafe
        if (jsonItem.containsKey(DEVICE_MANAGER_JSON_NAME_BUS) == false) { lastError += "isValid_JsonOneWireTemp_Item: jsonItem.containsKey(DEVICE_MANAGER_JSON_NAME_BUS) == false\n"; return false; }
        if (jsonItem.containsKey(DEVICE_MANAGER_JSON_NAME_ROMID) == false) { lastError += "isValid_JsonOneWireTemp_Item: jsonItem.containsKey(DEVICE_MANAGER_JSON_NAME_ROMID) == false\n"; return false; }

        // allow uid to be both a integer-number or as a string interprented as a "8 nibble ascii hex number"(32bit value)
        if (jsonItem[DEVICE_MANAGER_JSON_NAME_BUS].is<int>()) { 
            if (bus!=nullptr) *bus = jsonItem[DEVICE_MANAGER_JSON_NAME_BUS].as<int>();
        }
        else if (jsonItem[DEVICE_MANAGER_JSON_NAME_BUS].is<const char*>()){
            const char* bus_asciiHexString = jsonItem[DEVICE_MANAGER_JSON_NAME_BUS].as<const char*>();
            if (bus_asciiHexString == nullptr) { lastError += "isValid_JsonOneWireTemp_Item: bus_asciiHexString == nullptr\n"; return false; }
            if (bus!=nullptr) *bus = static_cast<uint32_t>(std::strtoul(bus_asciiHexString, nullptr, 16));
        }
        else {
            if (bus!=nullptr) *bus = 0;
            lastError += "isValid_JsonOneWireTemp_Item: bus uid is either a number or a ascii hex value\n";
            return false;
        }

        romid = jsonItem[DEVICE_MANAGER_JSON_NAME_ROMID].as<const char*>();
        return romid != nullptr;
    }

    bool readJson()
    {
        lastError = "";
        if (!LittleFS.exists(DEVICE_MANAGER_FILES_PATH))
        {
            LittleFS.mkdir(DEVICE_MANAGER_FILES_PATH);
            lastError = "dir did not exist";
            return false;
        }
        if (LittleFS.exists(DEVICE_MANAGER_CONFIG_JSON_FILE) == false) {
            lastError = "cfg file did not exist";
            return false;
        }
        int size = LittleFS_ext::getFileSize(DEVICE_MANAGER_CONFIG_JSON_FILE);
        char jsonBuffer[size + 1]; // +1 for null char
        if (LittleFS_ext::load_from_file(DEVICE_MANAGER_CONFIG_JSON_FILE, jsonBuffer) == false)
        {
            lastError = "error could not load json file";
            return false;
        }
        DynamicJsonDocument jsonDoc(size*2);
        DeserializationError error = deserializeJson(jsonDoc, jsonBuffer);
        if (error)
        {
            lastError = "deserialization failed: " + std::string(error.c_str());
            return false;
        }
        DEBUG_UART.print("jsonDoc.memoryUsage="); DEBUG_UART.print(jsonDoc.memoryUsage()); DEBUG_UART.print(" of "); DEBUG_UART.println(jsonDoc.capacity());
        if (!jsonDoc.is<JsonArray>()) {
            lastError = "jsonDoc root is not a JsonArray\n";
            return false;
        }
        lastError = "";
        JsonArray jsonItems = jsonDoc.as<JsonArray>();
        if (jsonItems == nullptr) {
            lastError = "jsonDoc root could not convert to a JsonArray\n";
            return false;
        }
        int newDeviceCount = 0;
        int newOneWireBusCount = 0;
        // *** first count valid devices ***
        int jsonItemCount = jsonItems.size();
        for (int di=0;di<jsonItemCount;di++) {
            const char* type = nullptr;
            if (isValid_JsonDevice_Item(jsonItems[di], type) == false) continue;
            if (strncmp(type, DEVICE_MANAGER_JSON_NAME_TYPE_ONE_WIRE_BUS, sizeof(DEVICE_MANAGER_JSON_NAME_TYPE_ONE_WIRE_BUS)-1) == 0)
            {
                if (isValid_JsonOneWireBus_Item(jsonItems[di]) == false) continue;
                newOneWireBusCount++;
            }
            else
            {
                newDeviceCount++;
            }
        }
        // *** second add oneWireBusses as they are needed for lockup of onewire devices pins ***
        // cleanup
        if (oneWireBusses != nullptr)
        {
            delete[] oneWireBusses;
            oneWireBusses = nullptr;
        }
        oneWireBusCount = newOneWireBusCount;
        oneWireBusses = new OneWireBus[oneWireBusCount];
        if (oneWireBusses == nullptr){ lastError="could not allocate memory for onewirebusses list"; return false; }
        int currIndex = 0;
        for (int di = 0; di < jsonItemCount && currIndex < oneWireBusCount; di++) {
            const char* type = nullptr;
            uint32_t uid = 0;
            if (isValid_JsonDevice_Item(jsonItems[di], type, &uid) == false) { lastError += "isValid_JsonDevice_Item == false\n"; continue; }
            if (strncmp(type, DEVICE_MANAGER_JSON_NAME_TYPE_ONE_WIRE_BUS, sizeof(DEVICE_MANAGER_JSON_NAME_TYPE_ONE_WIRE_BUS)-1) == 0)
            {
                if (isValid_JsonOneWireBus_Item(jsonItems[di]) == false) continue;
                // here isValid_JsonOneWireBusItem have verified that the values can be retreived safely
                oneWireBusses[currIndex].uid = uid;
                oneWireBusses[currIndex].pin = jsonItems[di][DEVICE_MANAGER_JSON_NAME_PIN].as<int>();
                currIndex++;
            }
        }

        // *** third add all endpoint devices ***
        // cleanup of prev device list if existent
        if (devices != nullptr) { 
            for (int i=0;i<deviceCount;i++) {
                if (devices[i] != nullptr) {
                    delete devices[i];
                    devices[i] = nullptr;
                }
            }
            delete[] devices;
            devices = nullptr;
        }
        deviceCount = newDeviceCount;
        devices = new Device*[deviceCount];
        if (devices == nullptr) { lastError="could not allocate memory for device list"; return false; }
        // Initialize all pointers to nullptr
        for (int di=0;di<deviceCount;di++)
            devices[di] = nullptr;

        currIndex = 0;
        for (int di = 0; di < jsonItemCount && currIndex < deviceCount; di++) {
            const char* type = nullptr;
            uint32_t uid = 0;
            JsonVariant jsonItem = jsonItems[di];
            if (isValid_JsonDevice_Item(jsonItem, type, &uid) == false) continue;

            if (strncmp(type, DEVICE_MANAGER_JSON_NAME_TYPE_ONE_WIRE_TEMP, sizeof(DEVICE_MANAGER_JSON_NAME_TYPE_ONE_WIRE_TEMP)-1) == 0)
            {
                const char* romid = nullptr;
                uint32_t busUid = 0;
                if (isValid_JsonOneWireTemp_Item(jsonItem, romid, &busUid) == false) { devices[currIndex++] = nullptr; lastError+="not valid JsonOneWireTemp Item\n"; continue; }
                // here isValid_JsonOneWireTempItem have verified that the values can be retreived safely

                int pin = getOneWireBusPin(busUid);
                if (pin == -1) { devices[currIndex++] = nullptr; lastError += "could not get onewire bus pin\n"; continue; }
                devices[currIndex++] = new OneWireTempDevice(uid, pin, romid);
            }
            else if (strncmp(type, DEVICE_MANAGER_JSON_NAME_TYPE_DHT, sizeof(DEVICE_MANAGER_JSON_NAME_TYPE_DHT)-1) == 0)
            {
                const char* dhtTypeStr = nullptr;
                if (isValid_JsonDHT_Item(jsonItem, dhtTypeStr) == false) { devices[currIndex++] = nullptr; continue; }
                // here isValid_JsonDHT_Item have verified that the values can be retreived safely
                int pin = jsonItem[DEVICE_MANAGER_JSON_NAME_PIN].as<int>();

                devices[currIndex++] = new DHTdevice(dhtTypeStr, uid, pin);
            }
            else if (strncmp(type, DEVICE_MANAGER_JSON_NAME_TYPE_PWM, sizeof(DEVICE_MANAGER_JSON_NAME_TYPE_PWM)-1) == 0)
            {
                // TODO add isValid_JsonPWM_Item function
                int pin = jsonItem[DEVICE_MANAGER_JSON_NAME_PIN].as<int>();
                int freq = jsonItem[DEVICE_MANAGER_JSON_NAME_PWM_FREQ].as<int>();
                int bits = jsonItem[DEVICE_MANAGER_JSON_NAME_PWM_BITS].as<int>();
                int invOut = jsonItem[DEVICE_MANAGER_JSON_NAME_PWM_INV_OUT].as<int>();
                devices[currIndex++] = new PWMdevice(uid, pin, freq, bits, invOut);
            }
            else if (strncmp(type, DEVICE_MANAGER_JSON_NAME_TYPE_TX433, sizeof(DEVICE_MANAGER_JSON_NAME_TYPE_TX433)-1) == 0)
            {
                // TODO add isValid_JsonTX433_Item function
                int pin = jsonItem[DEVICE_MANAGER_JSON_NAME_PIN].as<int>();
                devices[currIndex++] = new TX433device(uid, pin);
            }
            else if (strncmp(type, DEVICE_MANAGER_JSON_NAME_TYPE_DOUT, sizeof(DEVICE_MANAGER_JSON_NAME_TYPE_DOUT)-1) == 0)
            {
                // TODO add isValid_JsonDOUT_Item function
                int pin = jsonItem[DEVICE_MANAGER_JSON_NAME_PIN].as<int>();
                devices[currIndex++] = new DOUTdevice(uid, pin);
            }
            else if (strncmp(type, DEVICE_MANAGER_JSON_NAME_TYPE_DIN, sizeof(DEVICE_MANAGER_JSON_NAME_TYPE_DIN)-1) == 0)
            {
                // TODO add isValid_JsonDIN_Item function
                int pin = jsonItem[DEVICE_MANAGER_JSON_NAME_PIN].as<int>();
                devices[currIndex++] = new DINdevice(uid, pin);
            }
            else
            {
                lastError += "invalid device type: ";
                lastError += type;
                lastError += "\n";
            }
        }

        return true;
    }

    void HtmlDebugPrintDevices()
    {
        String ret = "";
        ret.concat("OneWireBusses:<br>");
        if (oneWireBusses == nullptr) {
            ret.concat("No one busses defined!");
        }
        else {
            for (int owbi=0;owbi<oneWireBusCount;owbi++)
            {
                ret.concat("index="); ret.concat(owbi);
                ret.concat(", ");
                ret.concat(oneWireBusses[owbi].ToString());
                ret.concat("<br>");
            }
        }
        ret.concat("Devices:<br>");
        if (devices == nullptr) {
            ret.concat("No devices defined!");
        }
        else {
            for (int di=0;di<deviceCount;di++)
            {
                ret.concat("index="); ret.concat(di);
                Device* devicePtr = devices[di];
                if (devicePtr == nullptr)
                {
                    ret.concat(", device is null<br>");
                    continue;
                }
                ret.concat(", ");
                if (devicePtr->type == DeviceType::OneWireTemp) {
                    OneWireTempDevice& owtd = static_cast<OneWireTempDevice&>(*devicePtr);
                    ret.concat(owtd.ToString());
                }
                else if (devicePtr->type == DeviceType::DHT) {
                    DHTdevice& dhtd = static_cast<DHTdevice&>(*devicePtr);
                    ret.concat(dhtd.ToString());
                }
                else if (devicePtr->type == DeviceType::PWM) {
                    PWMdevice& pwmd = static_cast<PWMdevice&>(*devicePtr);
                    ret.concat(pwmd.ToString());
                }
                else if (devicePtr->type == DeviceType::TX433) {
                    TX433device& tx433d = static_cast<TX433device&>(*devicePtr);
                    ret.concat(tx433d.ToString());
                }
                else {
                    Device &device = *devicePtr;
                    ret.concat(device.ToString());
                }
                
                ret.concat("<br>");
            }
        }
        webserver->send(200, "text/html", ret);
    }

    void reloadJSON()
    {
        if (!webserver->chunkedResponseModeStart(200, "text/html")) {
            webserver->send(505, F("text/html"), F("HTTP1.1 required"));
            return;
        }
        webserver->sendContent("LOAD JSON start<br>");
        if (readJson())
            webserver->sendContent("LOAD JSON ok<br>");
        else {
            String message = "LOAD JSON fail<br>" + String(lastError.c_str());
            webserver->sendContent(message);
        }
        webserver->chunkedResponseFinalize();
    }
    void htmlGetValue()
    {
        if (webserver->hasArg("uid") == false) {
            webserver->send(200, "text/html", "uid parameter missing");
            return;
        }
        uint32_t uid = atoi(webserver->arg("uid").c_str());
        float value = 0;
        if (getValue(uid, &value) == false) 
        {
            webserver->send(200, "text/html", "could not get value");
            return;
        }
        String ret = "";
        ret.concat("value:");
        ret.concat(value);
        webserver->send(200, "text/html", ret);
    }

    void restAPI_handleWriteOrRead(AsyncWebServerRequest *request) {
        // Example URL: /write/pwm/tempSensor1/255

        String url = request->url(); // e.g., "/write/pwm/tempSensor1/255"

        // Remove the leading '/'
        if (url.startsWith("/")) url = url.substring(1);  // -> "write/pwm/tempSensor1/255"

        // Split into parts
        int p1 = url.indexOf('/');
        int p2 = url.indexOf('/', p1 + 1);
        int p3 = url.indexOf('/', p2 + 1);

        String command = url.substring(0, p1);                         // "write" or "read"
        String type = url.substring(p1 + 1, p2);                       // "pwm", "gpio", etc.
        String uid  = url.substring(p2 + 1, p3);                       // "tempSensor1"
        String value = url.substring(p3 + 1);                          // "255" or "" if missing

        // Do something with them
        String message = "";
        
        message += "\"debug\":{";
        message += "\"Command\":\"" + command + "\",";
        message += "\"Type\":\"" + type + "\",";
        message += "\"UID\":\"" + uid + "\",";
        message += "\"Value\":\"" + value + "\"},";


        if (command == DEVICE_MANAGER_REST_API_WRITE_CMD) {
            // Handle write command
            if (value.length() > 0 || p3 != -1) {
                if (type == DEVICE_MANAGER_REST_API_UINT32_TYPE) {
                    // Convert value to integer
                    int intValue = value.toInt();
                    uint32_t uidInt = (uint32_t) strtoul(uid.c_str(), nullptr, 16);
                    if (setValue(uidInt, intValue))
                        message += "\"message\":\"Value written: " + String(intValue) + "\"";
                    else
                        message += "\"error\":\"Failed to write value.\"";

                } else if (type == DEVICE_MANAGER_REST_API_STRING_TYPE) {
                    // Convert value to string
                    uint32_t uidInt = (uint32_t) strtoul(uid.c_str(), nullptr, 16);
                    if (setValue(uidInt, value.c_str()))
                        message += "\"message\":\"String written: " + value + "\"";
                    else
                        message += "\"error\":\"Failed to write string.\"";

                } else {
                    message += "\"error\":\"Unknown type for writing.\"";
                }
            } else {
                message += "\"error\":\"No value provided for writing.\"";
            }
        } else if (command == DEVICE_MANAGER_REST_API_READ_CMD) {
            if (type == DEVICE_MANAGER_REST_API_UINT32_TYPE) {
                // Handle read command
                uint32_t readValue = 0;
                uint32_t uidInt = (uint32_t) strtoul(uid.c_str(), nullptr, 16);
                if (getValue(uidInt, &readValue)) {
                    message += "\"value\":\"" + String(readValue) + "\"";
                } else {
                    message += "\"error\":\"Failed to read value.\"";
                }
            } else if (type == DEVICE_MANAGER_REST_API_FLOAT_TYPE) {
                // Handle read command
                float readValue = 0;
                uint32_t uidInt = (uint32_t) strtoul(uid.c_str(), nullptr, 16);
                if (getValue(uidInt, &readValue)) {
                    message += "\"value\":\"" + String(readValue) + "\"";
                } else {
                    message += "\"error\":\"Failed to read value.\"";
                }
            } else {
                message += "\"error\":\"Unknown type for reading.\"";
            }
        } else {
            message += "\"error\":\"Unknown command.\"";
        }

        request->send(200, "application/json", "{" +  message + "}");
    }


    void setup(WEBSERVER_TYPE &srv) {

        webserver = &srv;
        srv.on(DEVICE_MANAGER_URL_RELOAD_JSON, HTTP_GET, reloadJSON);
        srv.on(DEVICE_MANAGER_URL_GET_VALUE, HTTP_GET, htmlGetValue);
        srv.on(DEVICE_MANAGER_URL_LIST_ALL_1WIRE_TEMPS, HTTP_GET, htmlGetAllOneWireTemperatures);
        srv.on(DEVICE_MANAGER_URL_LIST_ALL_1WIRE_DEVICES, HTTP_GET, htmlGetListOfOneWireDevicesOnBusPin);
        srv.on(DEVICE_MANAGER_URL_PRINT_DEVICES, HTTP_GET, HtmlDebugPrintDevices);
        GPIO_manager::setup(srv);
        if (readJson() == false)
        {
            String message = "Device Manager - LOAD JSON fail<br>" + String(lastError.c_str());
            DEBUG_UART.println(message);
        }
        
        asyncWebserver = new AsyncWebServer(DEVICE_MANAGER_REST_API_PORT);
        asyncWebserver->on(DEVICE_MANAGER_REST_API_WRITE_URL "*", HTTP_ANY, restAPI_handleWriteOrRead);
        asyncWebserver->on(DEVICE_MANAGER_REST_API_READ_URL "*", HTTP_ANY, restAPI_handleWriteOrRead);

        asyncWebserver->on("/", HTTP_ANY, [](AsyncWebServerRequest *request){
            String message = "Device Manager - REST API<br>";
            message += "<a href=\"/<cmd>/<type>/<uid>/<optional_value>\">Device command</a><br>";
            request->send(200, "text/html", message);
        });
        asyncWebserver->onNotFound([](AsyncWebServerRequest *request){
            String message = "Device Manager - REST API<br>";
            message += "invalid url:" + request->url() + "<br>";
            request->send(404, "text/html", message);
        });
        asyncWebserver->begin();
    }

    Device* getDeviceInfo(uint32_t uid)
    {
        for (int i=0;i<deviceCount;i++)
        {
            if (devices[i] != nullptr && devices[i]->uid == uid)
                return devices[i];
        }
        return nullptr;
    }
    int getTotalCountOfOneWireDevices()
    {
        int count = 0;
        for (int i=0;i<deviceCount;i++)
        {
            if (devices[i] != nullptr && devices[i]->type == DeviceType::OneWireTemp)
                count++;
        }
        return count;
    }
    bool contains(int *array, size_t arraySize, uint8_t value)
    {
        for (int i=0;i<arraySize;i++)
        {
            if (array[i] == value)
                return true;
        }
        return false;
    }
#ifdef BUSSES_DEV_PRINT
    String getAllOneWireTemperatures_busses_debug_print = "";
#endif
    bool getAllOneWireTemperatures()
    {
        dTemp.setWaitForConversion(false);
#ifdef BUSSES_DEV_PRINT
        getAllOneWireTemperatures_busses_debug_print = "unique busses @ pins:[";// development test print
#endif
        for (int i=0;i<oneWireBusCount;i++)
        {
            oneWire.begin(oneWireBusses[i].pin);
            dTemp.requestTemperatures();
#ifdef BUSSES_DEV_PRINT
            getAllOneWireTemperatures_busses_debug_print.concat(oneWireBusses[i].pin);
            if (i<oneWireBusCount-1)
                getAllOneWireTemperatures_busses_debug_print.concat(",");
#endif
        }
        delay(800); // wait for conversion to complete

        for (int i=0;i<deviceCount;i++)
        {
            if (devices[i] == nullptr) continue;
            if (devices[i]->type != DeviceType::OneWireTemp) continue;
            OneWireTempDevice& owtd = static_cast<OneWireTempDevice&>(*devices[i]);
            if (owtd.romid == nullptr) continue;

            oneWire.begin(owtd.pin);
            owtd.value = dTemp.getTempC(owtd.romid);
        }
        return true;
    }
    void htmlGetAllOneWireTemperatures()
    {
        getAllOneWireTemperatures();
        String ret = "";
#ifdef BUSSES_DEV_PRINT
        ret.concat(getAllOneWireTemperatures_busses_debug_print);
#endif
        ret.concat("]<br>Device temperatures:<br>");
        for (int i=0;i<deviceCount;i++)
        {
            if (devices[i] == nullptr) continue;
            if (devices[i]->type != DeviceType::OneWireTemp) continue;
            OneWireTempDevice& owtd = static_cast<OneWireTempDevice&>(*devices[i]);
            ret.concat(owtd.ToString());
            ret.concat("<br>");
        }
        webserver->send(200, "text/html", ret);
    }

    void DecodeFromJSON(std::string jsonStr) {
        StaticJsonDocument<128> json;
        deserializeJson(json, jsonStr.c_str());
        if (json.isNull()) {
            DEBUG_UART.println("Failed to parse JSON");
            return;
        }
        if (json.containsKey("uid") == false) {
            DEBUG_UART.println("Missing 'uid' key in JSON");
            return;
        }
        if (json.containsKey("cmd") == false) {
            DEBUG_UART.println("Missing 'cmd' key in JSON");
            return;
        }
        if (json.containsKey("value") == false) {
            DEBUG_UART.println("Missing 'value' key in JSON");
            return;
        }
        if (json["uid"].isNull()) {
            DEBUG_UART.println("Missing 'uid' value in JSON");
            return;
        }
        if (json["cmd"].isNull()) {
            DEBUG_UART.println("Missing 'cmd' value in JSON");
            return;
        }
        if (json["value"].isNull()) {
            DEBUG_UART.println("Missing 'value' value in JSON");
            return;
        }
        std::string uid = json["uid"].as<std::string>();
        std::string cmd = json["cmd"].as<std::string>();
        std::string value = json["value"].as<std::string>();
        if (cmd == "writeuint32") {
            uint32_t uidInt = (uint32_t) strtoul(uid.c_str(), nullptr, 16);
            uint32_t valueInt = (uint32_t) strtoul(value.c_str(), nullptr, 10);
            setValue(uidInt, valueInt);
        }
        else if (cmd == "writefloat") {
            uint32_t uidInt = (uint32_t) strtoul(uid.c_str(), nullptr, 16);
            float valueFloat = std::stof(value);
            setValue(uidInt, valueFloat);
        }
        else if (cmd == "writeString") {
            uint32_t uidInt = (uint32_t) strtoul(uid.c_str(), nullptr, 16);
            setValue(uidInt, value);
        }
        else {
            DEBUG_UART.println("Unknown command in JSON");
            return;
        }
    }

    bool getValue(uint32_t uid, float* value)
    {
        if (value == nullptr) return false; // no point of doing anything if value ptr is null

        Device* device = getDeviceInfo(uid);
        if (device == nullptr) {
            *value = 0;
            DEBUG_UART.println("could not find the device info, make sure that it's defined in the json");
            return false;
        }
        if (device->type == DeviceType::OneWireTemp) {
            // this don't do anything as
            // it's ensured that getAllOneWireTemperatures has been executed beforehand
            // calling this function "getValue"
            OneWireTempDevice& owtd = static_cast<OneWireTempDevice&>(*device);
            *value = owtd.value;
        }
        else if (device->type == DeviceType::DHT) {
            DHTdevice& dhtd = static_cast<DHTdevice&>(*device);
            if (dhtd.dhtType == DHT_Type::DHT11)
                dht.setup(dhtd.pin, DHTesp::DHT11);
            else if (dhtd.dhtType == DHT_Type::DHT22)
                dht.setup(dhtd.pin, DHTesp::DHT22);
            else if (dhtd.dhtType == DHT_Type::AM2302)
                dht.setup(dhtd.pin, DHTesp::AM2302);
            else if (dhtd.dhtType == DHT_Type::RHT03)
                dht.setup(dhtd.pin, DHTesp::RHT03);
            else
            {
                *value = 0;
                return false;
            }
            dhtd.value = dht.getHumidity();
            *value = dhtd.value;
        }
        else {
            *value = 0;
            return false;
        }
        return true;
    }

    bool getValue(uint32_t uid, uint32_t* value)
    {
        if (value == nullptr) return false; // no point of doing anything if value ptr is null

        Device* device = getDeviceInfo(uid);
        if (device == nullptr) {
            *value = 0;
            DEBUG_UART.println("could not find the device info, make sure that it's defined in the json");
            return false;
        }
        if (device->type == DeviceType::ADC) {
            pinMode(device->pin, INPUT);
            *value = analogRead(device->pin);
        }
        else if (device->type == DeviceType::DIN) {
            pinMode(device->pin, INPUT);
            *value = digitalRead(device->pin);
        }
        // keep theese commented out as they are not implemented yet
        /*else if (device->type == DeviceType::PWM) {}*/ // cannot get value from a PWM output
        /*else if (device->type == DeviceType::DOUT) {}*/ // cannot get value from a digital output
        /*else if (device->type == DeviceType::TX433) {}*/ // don't have a value to get
        /*else if (device->type == DeviceType::DAC) {}*/ // cannot get value from a digital output
        /*else if (device->type == DeviceType::OneWireTemp) {}*/ // unsupported data type
        else {
            *value = 0;
            return false;
        }
        return true;
    }

    bool setValue(uint32_t uid, uint32_t value)
    {
        Device* device = getDeviceInfo(uid);
        if (device == nullptr) {
            DEBUG_UART.println("could not find the device info, make sure that it's defined in the json");
            return false;
        }
        if (device->type == DeviceType::PWM) {
            analogWrite(device->pin, value);
        }
        else if (device->type == DeviceType::DOUT) {
            pinMode(device->pin, OUTPUT);
            digitalWrite(device->pin, value);
        }
        else return false;

        return true;
    }

    bool setValue(uint32_t uid, std::string value) {
        Device* device = getDeviceInfo(uid);
        if (device == nullptr) {
            DEBUG_UART.println("could not find the device info, make sure that it's defined in the json");
            return false;
        }
        if (device->type == DeviceType::TX433) {
            TX433device& tx433d = static_cast<TX433device&>(*device);
            RF433::init(tx433d.pin);
            RF433::DecodeFromJSON(value);
        }
        else {
            DEBUG_UART.println("setValue: device type not supported");
            return false;
        }
        return true;
    }
    void htmlGetListOfOneWireDevicesOnBusPin()
    {
        uint8_t pin = 32;
        if (webserver->hasArg("pin"))
            pin = std::stoi(webserver->arg("pin").c_str());
        OneWire _1wire;
        _1wire.begin(pin);
        byte i = 0;
        byte done = 0;
        byte addr[8];
        String returnStr;
        char hexString[3];

        while(!done)
        {
            if (_1wire.search(addr) != 1)
            {
                returnStr.concat("<br>No more addresses.<br>");
                _1wire.reset_search();
                done = 1;
            }
            else
            {
                returnStr.concat("<br>Unique ID = ");
                for( i = 0; i < 7; i++) 
                {
                    sprintf(hexString, "%02X", addr[i]);
                    returnStr.concat(hexString);
                    returnStr.concat(":");
                }
                sprintf(hexString, "%02X", addr[7]);
                    returnStr.concat(hexString);
                returnStr.concat("<br>");
            }
        }
        webserver->send(200,F("text/html"), returnStr);
    }
}
