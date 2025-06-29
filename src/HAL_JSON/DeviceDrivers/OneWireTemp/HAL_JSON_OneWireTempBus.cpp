
#include "HAL_JSON_OneWireTempBus.h"

namespace HAL_JSON {

    Device* OneWireTempBusAtRoot::Create(const JsonVariant &jsonObj) {
        return new OneWireTempBusAtRoot(jsonObj);
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
            if (item.is<const char*>() == false) continue; // comment item
            if (OneWireTempDevice::VerifyJSON(item) == false) HAL_JSON_VALIDATE_IN_LOOP_FAIL_OPERATION;
            validItemCount++;
        }
        if (validItemCount == 0) { GlobalLogger.Error(HAL_JSON_ERR_ITEMS_NOT_VALID("OneWireTempBus")); return false; }
        
        uint8_t pin = jsonObj[HAL_JSON_KEYNAME_PIN].as<uint8_t>(); 
        return GPIO_manager::CheckIfPinAvailableAndReserve(pin, (static_cast<uint8_t>(GPIO_manager::PinMode::OUT) | static_cast<uint8_t>(GPIO_manager::PinMode::IN)));
    }

    OneWireTempBus::OneWireTempBus(const JsonVariant &jsonObj) : Device(UIDPathMaxLength::Two) {
        pin = jsonObj[HAL_JSON_KEYNAME_PIN].as<uint8_t>();
        GPIO_manager::ReservePin(pin); // this is in most cases taken care of in OneWireTempBus::VerifyJSON but there are situations where it's needed

        oneWire = new OneWire(pin);
        dTemp = new DallasTemperature(oneWire);
        dTemp->setWaitForConversion(false);

        deviceCount = 0;
        JsonArray items = jsonObj[HAL_JSON_KEYNAME_ITEMS].as<JsonArray>();
        uint32_t itemCount = items.size();
        bool* validDevices = new bool[itemCount];
        // first pass count valid devices
        for (int i=0;i<itemCount;i++) {
            bool valid = OneWireTempDevice::VerifyJSON(items[i]);
            validDevices[i] = valid;
            if (valid == false) continue;
            deviceCount++;
        }
        devices = new (std::nothrow) OneWireTempDevice*[deviceCount];
        uint32_t index = 0;
        for (int i=0;i<itemCount;i++) {
            if (validDevices[i] == false) continue;
            devices[index++] = new OneWireTempDevice(static_cast<const JsonVariant&>(items[i]));
        }
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
                devices[i]->value = dTemp->getTempC(devices[i]->romid);
            else if (devices[i]->format == OneWireTempDeviceTempFormat::Fahrenheit)
                devices[i]->value = dTemp->getTempF(devices[i]->romid);
        }
    }

    Device* OneWireTempBus::findDevice(UIDPath& path) {
        uint64_t currLevelUID = 0;
        if (uid != 0) // current device uid
            currLevelUID = path.peekNextUID();
        else  // current device uid == 0
            currLevelUID = path.getCurrentUID();
        if (currLevelUID == UIDPath::UID_INVALID) return nullptr; // early break

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
            return false; // currently not implemented
        }
        else if (val.cmd == F("getAllDevices")) { // (as json) return a complete list of all devices found for all busses
            val.out_value += getAllDevices();
            return true; // currently not implemented
        }
        else if (val.cmd == F("getAllTemperatures")) { // (as json) return a complete list of all temperatures each with it's uid as the keyname and the temp as the value

            return false; // currently not implemented
        }
        val.out_value = F("{\"error\":\"cmd not found\"}");
        return true;  // cmd not found
    }

    String OneWireTempBus::getAllDevices() {
        byte i = 0;
        byte done = 0;
        byte addr[8];
        String returnStr;
        char hexString[3];
        uint32_t count = 0;
        returnStr.concat(F("{\"pin\":"));
        returnStr.concat(pin);
        returnStr.concat(F(",\"items\":[\"\"")); // plus one first dummy item so that 
        while(!done)
        {
            if (oneWire->search(addr) != 1)
            {
                
                oneWire->reset_search();
                done = 1;
            }
            else
            {
                returnStr.concat("\"");
                for( i = 0; i < 7; i++) 
                {
                    sprintf(hexString, "%02X", addr[i]);
                    returnStr.concat(hexString);
                    returnStr.concat(":");
                }
                sprintf(hexString, "%02X", addr[7]);
                    returnStr.concat(hexString);
                returnStr.concat("\",");
                count++;
            }
        }
        returnStr.concat(F("\"\"]}")); // plus one dummy item so that , can be used between items
        return returnStr;
    }

    OneWireTempBusAtRoot::OneWireTempBusAtRoot(const JsonVariant &jsonObj) 
    : OneWireTempBus(jsonObj),
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

}