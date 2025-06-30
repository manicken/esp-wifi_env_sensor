
#include "HAL_JSON_OneWireTempBus.h"

namespace HAL_JSON {

    Device* OneWireTempBusAtRoot::Create(const JsonVariant &jsonObj, const char* type) {
        return new OneWireTempBusAtRoot(jsonObj, type);
    }

    bool OneWireTempBus::VerifyJSON(const JsonVariant &jsonObj) {
        if (jsonObj.containsKey(HAL_JSON_KEYNAME_PIN) == false) { GlobalLogger.Error(HAL_JSON_ERR_MISSING_KEY(HAL_JSON_KEYNAME_PIN)); return false; }
        if (jsonObj[HAL_JSON_KEYNAME_PIN].is<uint8_t>() == false)  { GlobalLogger.Error(HAL_JSON_ERR_VALUE_TYPE(HAL_JSON_KEYNAME_PIN)); return false; }
        
        if (jsonObj.containsKey(HAL_JSON_KEYNAME_ITEMS) == false) {
            GlobalLogger.Error(HAL_JSON_ERR_MISSING_KEY(HAL_JSON_KEYNAME_ITEMS));
            return false;
        }
        if (jsonObj[HAL_JSON_KEYNAME_ITEMS].is<JsonArray>() == false) {
            GlobalLogger.Error(HAL_JSON_ERR_VALUE_TYPE(HAL_JSON_KEYNAME_ITEMS " not array"));
            return false;
        }
        const JsonArray items = jsonObj[HAL_JSON_KEYNAME_ITEMS].as<JsonArray>();
        if (items.size() == 0) { GlobalLogger.Error(HAL_JSON_ERR_ITEMS_EMPTY("OneWireTempBus")); return false;}
        size_t itemCount = items.size();
        size_t validItemCount = 0;
        for (int i=0;i<itemCount;i++) {
            const JsonVariant item = items[i];
            if (item.is<const char*>() == true) continue; // comment item
            if (Device::DisabledInJson(item) == true) continue; // disabled
            if (OneWireTempDevice::VerifyJSON(item) == false) HAL_JSON_VALIDATE_IN_LOOP_FAIL_OPERATION;
            validItemCount++;
        }
        if (validItemCount == 0) { GlobalLogger.Error(HAL_JSON_ERR_ITEMS_NOT_VALID("OneWireTempBus")); return false; }
        
        uint8_t pin = jsonObj[HAL_JSON_KEYNAME_PIN].as<uint8_t>(); 
        return GPIO_manager::CheckIfPinAvailableAndReserve(pin, (static_cast<uint8_t>(GPIO_manager::PinMode::OUT) | static_cast<uint8_t>(GPIO_manager::PinMode::IN)));
    }

    OneWireTempBus::OneWireTempBus(const JsonVariant &jsonObj, const char* type) : Device(UIDPathMaxLength::Two, type) {
        const char* uidStr = jsonObj[HAL_JSON_KEYNAME_UID].as<const char*>();
        uid = encodeUID(uidStr);
        pin = jsonObj[HAL_JSON_KEYNAME_PIN].as<uint8_t>();
        GPIO_manager::ReservePin(pin); // this is in most cases taken care of in OneWireTempBus::VerifyJSON but there are situations where it's needed

        oneWire = new OneWire(pin);
        dTemp = new DallasTemperature(oneWire);
        dTemp->setWaitForConversion(false);

        deviceCount = 0;
        const JsonArray& items = jsonObj[HAL_JSON_KEYNAME_ITEMS].as<JsonArray>();
        uint32_t itemCount = items.size();
        bool* validDevices = new bool[itemCount];
        // first pass count valid devices
        for (int i=0;i<itemCount;i++) {
            const JsonVariant& item = items[i];
            bool valid = true;
            if (item.is<const char*>() == true)  valid = false; // comment item
            if (valid && Device::DisabledInJson(item) == true) valid = false; // disabled
            if (valid)
                valid = OneWireTempDevice::VerifyJSON(items[i]);
            validDevices[i] = valid;
            if (valid == false) continue;
            deviceCount++;
        }
        devices = new (std::nothrow) OneWireTempDevice*[deviceCount];
        uint32_t index = 0;
        for (int i=0;i<itemCount;i++) {
            if (validDevices[i] == false) continue;
            devices[index++] = new OneWireTempDevice(static_cast<const JsonVariant&>(items[i]), type); // here type is not used so we just take it from bus
        }
        delete[] validDevices;
    }

    OneWireTempBus::~OneWireTempBus() {
        if (devices != nullptr) {
            for (int i=0;i<deviceCount;i++)
                delete devices[i];
        }
        delete dTemp;
        delete oneWire;
        delete[] devices;
        devices = nullptr;
        pinMode(pin, INPUT); // set to default input
    }

    void OneWireTempBus::requestTemperatures()
    {
        dTemp->requestTemperatures();
    }

    void OneWireTempBus::readAll()
    {
        for (int i=0;i<deviceCount;i++) {
            if (devices[i]->format == OneWireTempDeviceTempFormat::Celsius)
                devices[i]->value = dTemp->getTempC(devices[i]->romid.bytes);
            else if (devices[i]->format == OneWireTempDeviceTempFormat::Fahrenheit)
                devices[i]->value = dTemp->getTempF(devices[i]->romid.bytes);
        }
    }

    Device* OneWireTempBus::findDevice(UIDPath& path) {
        uint64_t currLevelUID = 0;
        if (uid != 0) // current device uid
            currLevelUID = path.peekNextUID();
        else  // current device uid == 0
            currLevelUID = path.getCurrentUID();

        if (currLevelUID == UIDPath::UID_INVALID) { GlobalLogger.Error(F("OneWireTempBus::findDevice - currLevelUID == UIDPath::UID_INVALID")); return nullptr; } // early break
        
        //HAL_JSON_DEBUG(F("OneWireTempBus::findDevice - uid: "), decodeUID(uid).c_str());
        //HAL_JSON_DEBUG(F("OneWireTempBus::findDevice - currLevelUID: "), decodeUID(currLevelUID).c_str());
        
        for (int i=0;i<deviceCount;i++)
        {
            OneWireTempDevice* device = devices[i];
            if (!device) continue; // absolute failsafe
            if (device->uid == currLevelUID) return device;
        }
        return nullptr;
    }

    bool OneWireTempBus::read(const HALReadStringRequestValue& val) {
        if (val.cmd == F("getAllNewDevices")) { // (as json) return a list of all new devices found for all busses (this will compare against the current ones and only print new ones)
            val.out_value += getAllDevices(false, true);
            return true;
        }
        else if (val.cmd == F("getAllNewDevicesWithTemp")) { // (as json) return a list of all new devices found for all busses (this will compare against the current ones and only print new ones)
            val.out_value += getAllDevices(true, true);
            return true;
        }
        else if (val.cmd == F("getAllDevices")) { // (as json) return a complete list of all devices found for all busses
            val.out_value += getAllDevices(false, false);
            return true;
        }
        else if (val.cmd == F("getAllTemperatures")) { // (as json) return a complete list of all temperatures each with it's uid as the keyname and the temp as the value
            val.out_value += getAllDevices(true, false);
            return true;
        }
        val.out_value = F("{\"error\":\"cmd not found\"}");
        return true;
    }

    bool OneWireTempBus::haveDeviceWithRomID(OneWireAddress addr) {
        if (deviceCount == 0 || devices == nullptr) return false;
        for (int i=0;i<deviceCount;i++) {
            // cast the romid to uint64_t for fast and easy compare
            if (devices[i]->romid.id == addr.id) return true;
        }
        return false;
    }

    String OneWireTempBus::getAllDevices(bool printTemp, bool onlyNewDevices) {
        byte i = 0;
        byte done = 0;
        OneWireAddress addr;
        String returnStr;
        char hexString[3];
        bool first = true;

        returnStr.concat(F("{\"pin\":"));
        returnStr.concat(pin);
        returnStr.concat(F(",\"items\":["));
        if (printTemp) {
            dTemp->setWaitForConversion(true);
            dTemp->requestTemperatures();
            dTemp->setWaitForConversion(false);
        }
        
        oneWire->reset_search();
        while(!done)
        {
            if (oneWire->search(addr.bytes) != 1)
            {
                
                oneWire->reset_search();
                done = 1;
            }
            else
            {
                if (onlyNewDevices && haveDeviceWithRomID(addr)==true) continue;

                if (!first) {
                    returnStr.concat(F(","));
                }
                if (printTemp) {
                    returnStr.concat("{\"romId\":");
                }
                returnStr.concat("\"");
                for( i = 0; i < 7; i++) 
                {
                    sprintf(hexString, "%02X", addr.bytes[i]);
                    returnStr.concat(hexString);
                    returnStr.concat(":");
                }
                sprintf(hexString, "%02X", addr.bytes[7]);
                returnStr.concat(hexString);
                returnStr.concat("\"");
                if (printTemp) {
                    returnStr.concat(",\"tempC\":");
                    returnStr.concat(dTemp->getTempC(addr.bytes));
                    returnStr.concat(",\"tempF\":");
                    returnStr.concat(dTemp->getTempF(addr.bytes));
                    returnStr.concat("}");
                }
                first = false;
            }
        }
        
        returnStr.concat(F("]}"));
        return returnStr;
    }

    String OneWireTempBus::ToString() {
        String ret;
        ret += "\"pin\":" + String(pin);
        ret += ",\"devices\":[";
        for (int i=0;i<deviceCount;i++) {
            ret += "{" + devices[i]->ToString() + "}"; 
            if (i<deviceCount-1) ret += ",";
        }
        ret += "]";
        return ret;
    }

    OneWireTempBusAtRoot::OneWireTempBusAtRoot(const JsonVariant &jsonObj, const char* type) 
    : OneWireTempBus(jsonObj, type),
        autoRefresh(
              [this]() { requestTemperatures(); },
              [this]() { readAll(); })
    {
        autoRefresh.SetRefreshTimeMs(OneWireTempAutoRefresh::ParseRefreshTimeMs(jsonObj));
    }

    OneWireTempBusAtRoot::~OneWireTempBusAtRoot() {
        
    }

    void OneWireTempBusAtRoot::loop() {
        autoRefresh.loop();
    }

    String OneWireTempBusAtRoot::ToString() {
        String ret;
        ret += "\"type\":\""  +String(type)+  "\"";
        ret += "," + autoRefresh.ToString();
        ret += "," + OneWireTempBus::ToString();
        return ret;
    }
}