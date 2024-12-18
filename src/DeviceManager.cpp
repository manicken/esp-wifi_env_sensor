#include "DeviceManager.h"

namespace DeviceManager
{
    std::string lastError;

    Device **devices = nullptr;
    uint32_t deviceCount = 0;
    OneWireBus *oneWireBusses = nullptr;
    int oneWireBusCount = 0;

#ifdef ESP8266
    ESP8266WebServer *server = nullptr;
#elif defined(ESP32)
    fs_WebServer *server = nullptr;
#endif

    DHTesp dht;
    OneWire oneWire;
    DallasTemperature dTemp(&oneWire);

    OneWireDevice::~OneWireDevice() {
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

    bool isValid_JsonDevice_Item(JsonVariant jsonItem, const char*& type) {
        // TODO use lastError
        if (jsonItem == nullptr) return false;
        if (!jsonItem.is<JsonObject>()) return false;
        if (!jsonItem.containsKey(DEVICE_MANAGER_JSON_NAME_TYPE)) return false;
        if (!jsonItem[DEVICE_MANAGER_JSON_NAME_TYPE].is<const char*>()) return false;
        
        type = jsonItem[DEVICE_MANAGER_JSON_NAME_TYPE].as<const char*>();
        return type != nullptr;
    }
    bool isValid_JsonOneWireBus_Item(JsonVariant jsonItem)
    {
        // TODO use lastError
        if (jsonItem == nullptr) return false; // failsafe
        if (jsonItem.is<JsonObject>() == false) return false; // failsafe
        if (jsonItem.containsKey(DEVICE_MANAGER_JSON_NAME_UID) == false) return false;
        if (jsonItem.containsKey(DEVICE_MANAGER_JSON_NAME_PIN) == false) return false;
        if (jsonItem[DEVICE_MANAGER_JSON_NAME_UID].is<int>() == false) return false;
        if (jsonItem[DEVICE_MANAGER_JSON_NAME_PIN].is<int>() == false) return false;
        return true;
    }
    bool isValid_JsonOneWireTemp_Item(JsonVariant jsonItem, const char*& romid)
    {
        // TODO use lastError
        if (jsonItem == nullptr) return false; // failsafe
        if (jsonItem.is<JsonObject>() == false) return false; // failsafe
        if (jsonItem.containsKey(DEVICE_MANAGER_JSON_NAME_UID) == false) return false;
        if (jsonItem.containsKey(DEVICE_MANAGER_JSON_NAME_BUS) == false) return false;
        if (jsonItem.containsKey(DEVICE_MANAGER_JSON_NAME_ROMID) == false) return false;
        if (jsonItem[DEVICE_MANAGER_JSON_NAME_UID].is<int>() == false) return false;
        if (jsonItem[DEVICE_MANAGER_JSON_NAME_BUS].is<int>() == false) return false;

        romid = jsonItem[DEVICE_MANAGER_JSON_NAME_ROMID].as<const char*>();
        return romid != nullptr;
    }
    bool isValid_JsonDHT_Item(JsonVariant jsonItem, const char*& dhtType)
    {
        // TODO use lastError
        if (jsonItem == nullptr) return false; // failsafe
        if (jsonItem.is<JsonObject>() == false) return false; // failsafe
        if (jsonItem.containsKey(DEVICE_MANAGER_JSON_NAME_UID) == false) return false;
        if (jsonItem.containsKey(DEVICE_MANAGER_JSON_NAME_PIN) == false) return false;
        if (jsonItem[DEVICE_MANAGER_JSON_NAME_UID].is<int>() == false) return false;
        if (jsonItem[DEVICE_MANAGER_JSON_NAME_PIN].is<int>() == false) return false;
        if (jsonItem.containsKey(DEVICE_MANAGER_JSON_NAME_DHT_TYPE) == false) return false;

        dhtType = jsonItem[DEVICE_MANAGER_JSON_NAME_DHT_TYPE].as<const char*>();
        return dhtType != nullptr;
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
            if (isValid_JsonDevice_Item(jsonItems[di], type) == false) continue;
            if (strncmp(type, DEVICE_MANAGER_JSON_NAME_TYPE_ONE_WIRE_BUS, sizeof(DEVICE_MANAGER_JSON_NAME_TYPE_ONE_WIRE_BUS)-1) == 0)
            {
                if (isValid_JsonOneWireBus_Item(jsonItems[di]) == false) continue;
                // here isValid_JsonOneWireBusItem have verified that the values can be retreived safely
                oneWireBusses[currIndex].uid = jsonItems[di][DEVICE_MANAGER_JSON_NAME_UID].as<int>();
                oneWireBusses[currIndex].pin = jsonItems[di][DEVICE_MANAGER_JSON_NAME_PIN].as<int>();
                currIndex++;
            }
        }

        // *** third add all endpoint devices ***
        // cleanup
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
        currIndex = 0;
        for (int di = 0; di < jsonItemCount && currIndex < deviceCount; di++) {
            const char* type = nullptr;
            if (isValid_JsonDevice_Item(jsonItems[di], type) == false) continue;
            if (strncmp(type, DEVICE_MANAGER_JSON_NAME_TYPE_ONE_WIRE_TEMP, sizeof(DEVICE_MANAGER_JSON_NAME_TYPE_ONE_WIRE_TEMP)-1) == 0)
            {
                const char* romid = nullptr;
                if (isValid_JsonOneWireTemp_Item(jsonItems[di], romid) == false) { devices[currIndex++] = nullptr; continue; }
                // here isValid_JsonOneWireTempItem have verified that the values can be retreived safely
                int busUid = jsonItems[di][DEVICE_MANAGER_JSON_NAME_BUS].as<int>();
                int pin = getOneWireBusPin(busUid);
                if (pin == -1) { devices[currIndex++] = nullptr; continue; }
                int uid = jsonItems[di][DEVICE_MANAGER_JSON_NAME_UID].as<int>();
                
                devices[currIndex++] = new OneWireTempDevice(uid, pin, romid);
            }
            else if (strncmp(type, DEVICE_MANAGER_JSON_NAME_TYPE_DHT, sizeof(DEVICE_MANAGER_JSON_NAME_TYPE_DHT)-1) == 0)
            {
                const char* dhtTypeStr = nullptr;
                if (isValid_JsonDHT_Item(jsonItems[di], dhtTypeStr) == false) { devices[currIndex++] = nullptr; continue; }
                // here isValid_JsonDHT_Item have verified that the values can be retreived safely
                int uid = jsonItems[di][DEVICE_MANAGER_JSON_NAME_UID].as<int>();
                int pin = jsonItems[di][DEVICE_MANAGER_JSON_NAME_PIN].as<int>();

                devices[currIndex++] = new DHTdevice(dhtTypeStr, uid, pin);
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
            if (devices[i].type != DeviceType::OneWireTemp) continue;
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
            if (devices[i].type != DeviceType::OneWireTemp) continue;
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
