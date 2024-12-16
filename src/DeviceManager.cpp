#include "DeviceManager.h"

namespace DeviceManager
{
    std::string lastError;

    Device *devices = nullptr;

    BaseDevice **devices_ = nullptr; // proposed new structure

    uint32_t deviceCount = 0;
    int oneWireBusCount = 0;
    uint8_t *oneWireBusPins = nullptr;

#ifdef ESP8266
    ESP8266WebServer *server = nullptr;
#elif defined(ESP32)
    fs_WebServer *server = nullptr;
#endif

    DHTesp dht;
    OneWire oneWire;
    DallasTemperature dTemp(&oneWire);

    Device::~Device() {
        if (romid != nullptr)
            delete[] romid;
            romid = nullptr;
    }

    char ConvertOneNibble(uint8_t value)
    {
        if (value>9) return (value - 10) + 'A';
        else return value + '0';
    }

    std::string ByteToHexString(uint8_t value)
    {
        std::string hexString(2,'0');
        hexString[0] = ConvertOneNibble((value >> 4) & 0x0F);
        hexString[1] = ConvertOneNibble(value & 0x0F);
        return hexString;
    }

    std::string ByteArrayToString(uint8_t* byteArray, size_t arraySize)
    {
        std::string str = "";
        for (int i=0;i<arraySize;i++) {
            str.append(ByteToHexString(byteArray[i]));
            if (i<(arraySize-1)) str.append(":");
        }
        return str;
    }

    bool convertHexToBytes(const char* hexString, uint8_t* byteArray, size_t arraySize)
    {
        if (!hexString || !byteArray ) {
            DEBUG_UART.println("convertHexToBytes - Invalid input 1");
            return false; // Invalid input 1
        }
        size_t hexStrLen = strlen(hexString);
        int incr = 0;
        if (hexStrLen == arraySize*2) // no deliminator between hex-byte numbers
            incr = 2;
        else if (hexStrLen == (arraySize*3)-1)// any desired deliminator between hex-byte numbers
            incr = 3;
        else {
            DEBUG_UART.println("convertHexToBytes - hexStrLen mismatch:" + String(hexStrLen));
            return false; // Invalid input 2
        }
        size_t byteIndex = 0;
        for (size_t i = 0; i < hexStrLen; i+=incr) {
            if (byteIndex >= arraySize) {
                DEBUG_UART.println("Exceeded array size");
                return false; // Exceeded array size
            }
            char highNibble = hexString[i];
            char lowNibble = hexString[i + 1];

            if (!std::isxdigit(highNibble) || !std::isxdigit(lowNibble)) {
                DEBUG_UART.println("Non-hex character found");
                return false; // Non-hex character found
            }

            byteArray[byteIndex++] = (uint8_t)((std::isdigit(highNibble) ? highNibble - '0' : std::toupper(highNibble) - 'A' + 10) << 4 |
                                    (std::isdigit(lowNibble) ? lowNibble - '0' : std::toupper(lowNibble) - 'A' + 10));
        }

        return true; // Conversion successful
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
        DynamicJsonDocument jsonDoc(size);
        DeserializationError error = deserializeJson(jsonDoc, jsonBuffer);
        if (error)
        {
            lastError = "deserialization failed: " + std::string(error.c_str());
            return false;
        }
        if (!jsonDoc.is<JsonObject>()) {
            lastError = "jsonDoc is not a valid JsonObject\n";
            return false;
        }
        deviceCount = jsonDoc.size();
        if (devices != nullptr) { delete[] devices; devices = nullptr;}
        //devices = (Device*)malloc(sizeof(Device) * deviceCount);
        devices = new Device[deviceCount];
        if (devices == nullptr){ lastError="could not allocate memory"; return false; }

        int currIndex = 0;
        lastError = "";
        for (JsonPair kv : jsonDoc.as<JsonObject>()) {
            Device &device = devices[currIndex++];
            const char* uid = kv.key().c_str();
            
            if (!kv.value().is<JsonObject>()) {
                lastError += "item is not JSON object @ index:" + std::to_string(currIndex) + "\n";
                device.type = DeviceType::Unknown;
                continue;
            }

            JsonObject item = kv.value().as<JsonObject>();
            if (!item.containsKey("type") || !item.containsKey("pin")) {
                lastError += "item missing required keys @ index:" + std::to_string(currIndex) + "\n";
                device.type = DeviceType::Unknown;
                continue;
            }

            const char* type = item["type"].as<const char*>();
            if (!item["pin"].is<int>()) {
                lastError += "pin is not an integer @ index:" + std::to_string(currIndex) + "\n";
                device.type = DeviceType::Unknown;
                continue;
            }

            device.uid = atoi(uid);
            device.pin = item["pin"].as<int>();

            if (strncmp(type, "1wire", 7) == 0) device.type = DeviceType::OneWire;
            else if (strncmp(type, "DHT11", 5) == 0) device.type = DeviceType::DHT11;
            else if (strncmp(type, "DHT22", 5) == 0) device.type = DeviceType::DHT22;
            else if (strncmp(type, "DHT_AM2302", 10) == 0) device.type = DeviceType::DHT_AM2302;
            else if (strncmp(type, "DHT_RHT03", 9) == 0) device.type = DeviceType::DHT_RHT03;
            else if (strncmp(type, "FAN", 3) == 0) device.type = DeviceType::FAN;
            else if (strncmp(type, "RF433", 5) == 0) device.type = DeviceType::RF433;
            else device.type = DeviceType::Unknown;

            //DEBUG_UART.print("Type:"); DEBUG_UART.println((int)device.type);

            if (item.containsKey("romid")) {
                const char* romidStr = item["romid"].as<const char*>();
                device.romid = new uint8_t[8]();
                if (convertHexToBytes(romidStr, device.romid, 8) == false) { delete[] device.romid; device.romid = nullptr;}
                //device.romid = strdup(romid);
                //DEBUG_UART.print("RomId:"); DEBUG_UART.println(ByteArrayToString(device.romid, 8).c_str());
            } else {
                device.romid = nullptr;
            }

            /*if (item.containsKey("description")) {
                const char* descr = item["description"].as<const char*>();
                DEBUG_UART.print("Description:"); DEBUG_UART.println(descr);
            }*/
        }

        // the following makes a list of unique onewire bus pins
        // without the use of vector
        int oneWireDeviceCount = getTotalCountOfOneWireDevices();
        if (oneWireDeviceCount == 0) {
            
            return false;
        }
        int oneWireBusPins_temp[oneWireDeviceCount];
        for (int i=0;i<oneWireDeviceCount;i++)
            oneWireBusPins_temp[i] = -1;
        int cbi = 0; // current bus index
        for (int i=0;i<deviceCount;i++)
        {
            if (devices[i].type != DeviceType::OneWire) continue;

            if (contains(oneWireBusPins_temp, cbi, devices[i].pin) == false)
                oneWireBusPins_temp[cbi++] = devices[i].pin;
        }
        oneWireBusCount = 0;
        for (int i=0;i<oneWireDeviceCount;i++)
            if (oneWireBusPins_temp[i] != -1) oneWireBusCount++;
        if (oneWireBusPins != nullptr) delete[] oneWireBusPins;
        oneWireBusPins = new uint8_t[oneWireBusCount];
        int currOneWireBusPin = 0;
        for (int i=0;i<oneWireDeviceCount;i++)
            if (oneWireBusPins_temp[i] != -1) oneWireBusPins[currOneWireBusPin++] = oneWireBusPins_temp[i];

        return true;
    }

    void HtmlDebugPrintDevices()
    {
        String ret = "Devices:<br>";
        if (devices != nullptr) {
            for (int di=0;di<deviceCount;di++)
            {
                Device &device = devices[di];
                int type = (int)device.type;
                ret.concat("type="); ret.concat(type);
                ret.concat(", pin="); ret.concat(device.pin);
                ret.concat(", uid="); ret.concat(device.uid);
                ret.concat(", fvalue="); ret.concat(device.fvalue);
                if (device.romid != nullptr) { ret.concat(", romId="); ret.concat(ByteArrayToString(device.romid, 8).c_str()); }
                ret.concat("<br>");
            }
        }
        else {
            ret = "No devices.";
        }
        server->send(200, "text/html", ret);
    }

    void reloadJSON()
    {
        if (!server->chunkedResponseModeStart(200, "text/html")) {
            server->send(505, F("text/html"), F("HTTP1.1 required"));
            return;
        }
        server->sendContent("LOAD JSON start<br>");
        if (readJson())
            server->sendContent("LOAD JSON ok<br>");
        else {
            String message = "LOAD JSON fail<br>" + String(lastError.c_str());
            server->sendContent(message);
        }
        server->chunkedResponseFinalize();
    }
    void htmlGetValue()
    {
        if (server->hasArg("uid") == false) {
            server->send(200, "text/html", "uid parameter missing");
            return;
        }
        uint32_t uid = atoi(server->arg("uid").c_str());
        float value = 0;
        if (getValue(uid, &value) == false) 
        {
            server->send(200, "text/html", "could not get value");
            return;
        }
        String ret = "";
        ret.concat("value:");
        ret.concat(value);
        server->send(200, "text/html", ret);
    }

#ifdef ESP8266
    void setup(ESP8266WebServer &srv) {
#elif defined(ESP32)
    void setup(fs_WebServer &srv) {
#endif
        server = &srv;
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
    }

    Device* getDeviceInfo(uint32_t uid)
    {
        for (int i=0;i<deviceCount;i++)
        {
            if (devices[i].uid == uid)
                return &devices[i];
        }
        return nullptr;
    }
    int getTotalCountOfOneWireDevices()
    {
        int count = 0;
        for (int i=0;i<deviceCount;i++)
        {
            if (devices[i].type == DeviceType::OneWire)
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
            oneWire.begin(oneWireBusPins[i]);
            dTemp.requestTemperatures();
#ifdef BUSSES_DEV_PRINT
            getAllOneWireTemperatures_busses_debug_print.concat(oneWireBusPins[i]);
            if (i<oneWireBusCount-1)
                getAllOneWireTemperatures_busses_debug_print.concat(",");
#endif
        }
        delay(800); // wait for conversion to complete

        for (int i=0;i<deviceCount;i++)
        {
            if (devices[i].type != DeviceType::OneWire) continue;
            if (devices[i].romid == nullptr) continue;

            oneWire.begin(devices[i].pin);
            devices[i].fvalue = dTemp.getTempC(devices[i].romid);
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
            if (devices[i].type != DeviceType::OneWire) continue;
            ret.concat("romid:"); ret.concat(ByteArrayToString(devices[i].romid, 8).c_str());
            ret.concat(", uid:"); ret.concat(devices[i].uid);
            ret.concat(", pin:"); ret.concat(devices[i].pin);
            ret.concat(", temp:"); ret.concat(devices[i].fvalue);
            ret.concat("<br>");
        }
        server->send(200, "text/html", ret);
    }
    /*
    TODO make it so that multiple devices can start to get it's value at the same time, thus make it non blocking for a lot of devices
    even 8 ds18b20 takes 6 sec to convert all
    this can be solved by start a conversion on all 1wire busses that is defined in the device list (json) at once
    then each value can be stored into the internal device list, and later be retreived one by one 
    by the higher layer (thingspeak for example)
    */
    bool getValue(uint32_t uid, float* value)
    {
        Device* device = getDeviceInfo(uid);
        if (device == nullptr) {
            if (value!=nullptr) *value = 0;
            DEBUG_UART.println("could not find the device info, make sure that it's defined in the json");
            return false;
        }
        if (device->type == DeviceType::OneWire) {
            // this don't do anything as
            // it's ensured that getAllOneWireTemperatures has been executed beforehand
            // calling this function "getValue"
        }
        else if (device->type == DeviceType::DHT11) {
            dht.setup(device->pin, DHTesp::DHT11);
            device->fvalue = dht.getHumidity();
        }
        else if (device->type == DeviceType::DHT22) {
            dht.setup(device->pin, DHTesp::DHT22);
            device->fvalue = dht.getHumidity();
        }
        else if (device->type == DeviceType::DHT_AM2302) {
            dht.setup(device->pin, DHTesp::AM2302);
            device->fvalue = dht.getHumidity();
        }
        else if (device->type == DeviceType::DHT_RHT03) {
            dht.setup(device->pin, DHTesp::RHT03);
            device->fvalue = dht.getHumidity();
        } 
        else {
            if (value!=nullptr) *value = 0;
            return false;
        }
        if (value==nullptr) return false;
        *value = device->fvalue;
        return true;
    }

    void setValue(uint32_t uid, float value)
    {

    }
    void setValue(uint32_t uid, uint32_t value)
    {
        
    }
    void htmlGetListOfOneWireDevicesOnBusPin()
    {
        uint8_t pin = 32;
        if (server->hasArg("pin"))
            pin = std::stoi(server->arg("pin").c_str());
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
        server->send(200,F("text/html"), returnStr);
    }
}
