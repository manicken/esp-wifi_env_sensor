#include "DeviceManager.h"

namespace DeviceManager
{
    Device *devices = nullptr;
    uint32_t deviceCount = 0;

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
            free(romid);
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
        if (!hexString || !byteArray || strlen(hexString) != 16 || arraySize < 8) {
            return false; // Invalid input
        }

        for (size_t i = 0; i < 8; ++i) {
            char highNibble = hexString[i * 2];
            char lowNibble = hexString[i * 2 + 1];

            if (!std::isxdigit(highNibble) || !std::isxdigit(lowNibble)) {
                return false; // Non-hex character found
            }

            byteArray[i] = (uint8_t)((std::isdigit(highNibble) ? highNibble - '0' : std::toupper(highNibble) - 'A' + 10) << 4 |
                                    (std::isdigit(lowNibble) ? lowNibble - '0' : std::toupper(lowNibble) - 'A' + 10));
        }

        return true; // Conversion successful
    }

    bool readJson()
    {
        DEBUG_UART.println("readJson func start");

        if (!LittleFS.exists(DEVICE_MANAGER_FILES_PATH))
        {
            LittleFS.mkdir(DEVICE_MANAGER_FILES_PATH);
            DEBUG_UART.println("Device Manager ERROR - dir did not exist");
            return false;
        }

        if (LittleFS.exists(DEVICE_MANAGER_CONFIG_JSON_FILE) == false) {
            DEBUG_UART.println("Device Manager ERROR - cfg file did not exist");
            return false;
        }

        DEBUG_UART.println("loading..");
        int size = LittleFS_ext::getFileSize(DEVICE_MANAGER_CONFIG_JSON_FILE);
        DEBUG_UART.print("File size:"); DEBUG_UART.println(size);

        char jsonBuffer[size + 1]; // +1 for null char
        if (LittleFS_ext::load_from_file(DEVICE_MANAGER_CONFIG_JSON_FILE, jsonBuffer) == false)
        {
            DEBUG_UART.println("error could not load file:");
            DEBUG_UART.println(DEVICE_MANAGER_CONFIG_JSON_FILE);
            return false;
        }

        DEBUG_UART.println("[OK]");
        DynamicJsonDocument jsonDoc(1024);
        DeserializationError error = deserializeJson(jsonDoc, jsonBuffer);
        DEBUG_UART.println("deserializing..");

        if (error)
        {
            DEBUG_UART.print("Device Manager ERROR - cfg Deserialization failed: ");
            DEBUG_UART.println(error.c_str());
            return false;
        }
        deviceCount = jsonDoc.size();
        DEBUG_UART.println("[OK]");
        DEBUG_UART.print("item count:"); DEBUG_UART.println(deviceCount);
        
        if (devices != nullptr) free(devices);
        devices = (Device*)malloc(sizeof(Device) * deviceCount);

        int currIndex = 0;
        for (JsonPair kv : jsonDoc.as<JsonObject>()) {
            Device &device = devices[currIndex++];
            const char* uid = kv.key().c_str();
            
            if (!kv.value().is<JsonObject>()) {
                DEBUG_UART.println("item is not JSON object");
                device.type = DeviceType::Unknown;
                continue;
            }

            JsonObject item = kv.value().as<JsonObject>();
            if (!item.containsKey("type") || !item.containsKey("pin")) {
                DEBUG_UART.println("item missing required keys");
                device.type = DeviceType::Unknown;
                continue;
            }

            const char* type = item["type"].as<const char*>();
            if (!item["pin"].is<int>()) {
                DEBUG_UART.println("pin is not an integer");
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

            DEBUG_UART.print("Type:"); DEBUG_UART.println((int)device.type);

            if (item.containsKey("romid")) {
                const char* romidStr = item["romid"].as<const char*>();
                device.romid = (uint8_t*)malloc(8);
                if (convertHexToBytes(romidStr, device.romid, 8) == false) { free(device.romid); device.romid = nullptr;}
                //device.romid = strdup(romid);
                DEBUG_UART.print("RomId:"); DEBUG_UART.println(ByteArrayToString(device.romid, 8).c_str());
            } else {
                device.romid = nullptr;
            }

            if (item.containsKey("description")) {
                const char* descr = item["description"].as<const char*>();
                DEBUG_UART.print("Description:"); DEBUG_UART.println(descr);
            }
        }

        return true;
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
        else
            server->sendContent("LOAD JSON fail<br>");
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
        uint8_t res = 0;
        if (getValue(uid, &value, &res) == false) 
        {
            server->send(200, "text/html", "could not get value");
            return;
        }
        String ret = "";
        ret.concat("value:");
        ret.concat(value);
        ret.concat(", @ "); ret.concat(res);
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
        srv.on(DEVICE_MANAGER_URL_GET_ALL_1WIRE_TEMPS, HTTP_GET, htmlGetAllOneWireTemperatures);
        GPIO_manager::setup(srv);
        readJson();
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
        int oneWireDeviceCount = getTotalCountOfOneWireDevices();
        if (oneWireDeviceCount == 0) {
            
            return false;
        }
        int busses[oneWireDeviceCount];
        for (int i=0;i<oneWireDeviceCount;i++)
            busses[i] = -1;
        int cbi = 0; // current bus index
        for (int i=0;i<deviceCount;i++)
        {
            if (devices[i].type != DeviceType::OneWire) continue;

            if (contains(busses, cbi, devices[i].pin) == false)
                busses[cbi++] = devices[i].pin;
        }
        dTemp.setWaitForConversion(false);
#ifdef BUSSES_DEV_PRINT
        getAllOneWireTemperatures_busses_debug_print = "unique busses @ pins:[";// development test print
#endif
        for (int i=0;i<oneWireDeviceCount;i++)
        {
            if (busses[i] == -1) continue;
            oneWire.begin(busses[i]);
            dTemp.requestTemperatures();
#ifdef BUSSES_DEV_PRINT
            getAllOneWireTemperatures_busses_debug_print.concat(busses[i]);
            if (i<oneWireDeviceCount-1)
                getAllOneWireTemperatures_busses_debug_print.concat(",");
#endif
        }
        delay(800); // wait for conversion to complete

        for (int i=0;i<deviceCount;i++)
        {
            if (devices[i].type != DeviceType::OneWire) continue;
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
    bool getValue(uint32_t uid, float* value, uint8_t* resolution)
    {
        Device* device = getDeviceInfo(uid);
        if (device == nullptr) {
            if (value!=nullptr) *value = 0;
            DEBUG_UART.println("could not find the device info, make sure that it's defined in the json");
            return false;
        }
        if (device->type == DeviceType::OneWire) {
            oneWire.begin(device->pin);
            DallasTemperature::request_t req = dTemp.requestTemperaturesByAddress(device->romid);
            if (resolution!=nullptr) *resolution = dTemp.getResolution(device->romid);
            if (req.result == false) { if (value!=nullptr) *value = 0; DEBUG_UART.println("dallas req.result == false"); return false; }
            if (value!=nullptr) *value = dTemp.getTempC(device->romid);
            else return false;
        }
        else if (device->type == DeviceType::DHT11) {
            dht.setup(device->pin, DHTesp::DHT11);
            if (value!=nullptr) *value = dht.getHumidity();
            else return false;
        }
        else if (device->type == DeviceType::DHT22) {
            dht.setup(device->pin, DHTesp::DHT22);
            if (value!=nullptr) *value = dht.getHumidity();
            else return false;
        }
        else if (device->type == DeviceType::DHT_AM2302) {
            dht.setup(device->pin, DHTesp::AM2302);
            if (value!=nullptr) *value = dht.getHumidity();
            else return false;
        }
        else if (device->type == DeviceType::DHT_RHT03) {
            dht.setup(device->pin, DHTesp::RHT03);
            if (value!=nullptr) *value = dht.getHumidity();
            else return false;
        } 
        else {
            if (value!=nullptr) *value = 0;
            return false;
        }
        return true;
    }

    void setValue(uint32_t uid, float value)
    {

    }
    void setValue(uint32_t uid, uint32_t value)
    {
        
    }
}
