
#include "HAL_JSON_OneWireTempBus.h"

namespace HAL_JSON {

    Device* OneWireTempBusAtRoot::Create(const JsonVariant &jsonObj, const char* type) {
        return new OneWireTempBusAtRoot(jsonObj, type);
    }

    bool OneWireTempBus::VerifyJSON(const JsonVariant &jsonObj) {
        
        if (jsonObj.containsKey(HAL_JSON_KEYNAME_ITEMS) == false) {
            GlobalLogger.Error(HAL_JSON_ERR_MISSING_KEY(HAL_JSON_KEYNAME_ITEMS));
            return false;
        }
        if (jsonObj[HAL_JSON_KEYNAME_ITEMS].is<JsonArray>() == false) {
            GlobalLogger.Error(HAL_JSON_ERR_VALUE_TYPE(HAL_JSON_KEYNAME_ITEMS " not array"));
            return false;
        }
        const JsonArray& items = jsonObj[HAL_JSON_KEYNAME_ITEMS].as<JsonArray>();
        if (items.size() == 0) { GlobalLogger.Error(HAL_JSON_ERR_ITEMS_EMPTY("OneWireTempBus")); return false;}
        int itemCount = (int)items.size();
        size_t validItemCount = 0;
        for (int i=0;i<itemCount;i++) {
            const JsonVariant item = items[i];
            if (IsConstChar(item) == true) continue; // comment item
            if (Device::DisabledInJson(item) == true) continue; // disabled
            if (OneWireTempDevice::VerifyJSON(item) == false) HAL_JSON_VALIDATE_IN_LOOP_FAIL_OPERATION;
            validItemCount++;
        }
        if (validItemCount == 0) { GlobalLogger.Error(HAL_JSON_ERR_ITEMS_NOT_VALID("OneWireTempBus")); return false; }
        
        //if (jsonObj.containsKey(HAL_JSON_KEYNAME_PIN) == false) { GlobalLogger.Error(HAL_JSON_ERR_MISSING_KEY(HAL_JSON_KEYNAME_PIN)); return false; }
        //if (jsonObj[HAL_JSON_KEYNAME_PIN].is<uint8_t>() == false)  { GlobalLogger.Error(HAL_JSON_ERR_VALUE_TYPE(HAL_JSON_KEYNAME_PIN)); return false; }
        
        //uint8_t pin = jsonObj[HAL_JSON_KEYNAME_PIN].as<uint8_t>(); 
        //return GPIO_manager::CheckIfPinAvailableAndReserve(pin, (static_cast<uint8_t>(GPIO_manager::PinMode::OUT) | static_cast<uint8_t>(GPIO_manager::PinMode::IN)));
        return GPIO_manager::ValidateJsonAndCheckIfPinAvailableAndReserve(jsonObj, (static_cast<uint8_t>(GPIO_manager::PinMode::OUT) | static_cast<uint8_t>(GPIO_manager::PinMode::IN)));
    }

    OneWireTempBus::OneWireTempBus(const JsonVariant &jsonObj, const char* type) : Device(UIDPathMaxLength::Two, type) {
        const char* uidStr = GetAsConstChar(jsonObj,HAL_JSON_KEYNAME_UID);//].as<const char*>();
        uid = encodeUID(uidStr);
        pin = GetAsUINT32(jsonObj,HAL_JSON_KEYNAME_PIN);//].as<uint8_t>();
        GPIO_manager::ReservePin(pin); // this is in most cases taken care of in OneWireTempBus::VerifyJSON but there are situations where it's needed

        oneWire = new OneWire(pin);
        dTemp = new DallasTemperature(oneWire);
        dTemp->setWaitForConversion(false);

        deviceCount = 0;
        const JsonArray& items = jsonObj[HAL_JSON_KEYNAME_ITEMS].as<JsonArray>();
        int itemCount = items.size();
        bool* validDevices = new bool[itemCount];
        // first pass count valid devices
        for (int i=0;i<itemCount;i++) {
            const JsonVariant& item = items[i];
            if (IsConstChar(item) == true) { validDevices[i] = false; continue; } // comment item
            if (Device::DisabledInJson(item) == true) { validDevices[i] = false; continue; } // disabled
            bool valid = OneWireTempDevice::VerifyJSON(items[i]);
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
        HAL_UID currLevelUID;
        if (uid.IsSet()) // current device uid
            currLevelUID = path.peekNextUID();
        else  // current device uid == 0
            currLevelUID = path.getCurrentUID();

        if (currLevelUID.Invalid()) { GlobalLogger.Error(F("OneWireTempBus::findDevice - currLevelUID is Invalid")); return nullptr; } // early break
        
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

    HALDeviceOperationResult OneWireTempBus::read(const HALReadStringRequestValue& val) {
        if (val.cmd == "getAllNewDevices") { // (as json) return a list of all new devices found for all busses (this will compare against the current ones and only print new ones)
            val.out_value += getAllDevices(false, true);
            return HALDeviceOperationResult::Success;
        }
        else if (val.cmd == "getAllNewDevicesWithTemp") { // (as json) return a list of all new devices found for all busses (this will compare against the current ones and only print new ones)
            val.out_value += getAllDevices(true, true);
            return HALDeviceOperationResult::Success;
        }
        else if (val.cmd == "getAllDevices") { // (as json) return a complete list of all devices found for all busses
            val.out_value += getAllDevices(false, false);
            return HALDeviceOperationResult::Success;
        }
        else if (val.cmd == "getAllTemperatures") { // (as json) return a complete list of all temperatures each with it's uid as the keyname and the temp as the value
            val.out_value += getAllDevices(true, false);
            return HALDeviceOperationResult::Success;
        }
        std::string stdStrCmd = val.cmd.ToString();
        GlobalLogger.Warn(F("OneWireTempBus::read - cmd not found: "), stdStrCmd.c_str()); // this can then be read by getting the last entry from logger
        return HALDeviceOperationResult::UnsupportedCommand;
    }

    bool OneWireTempBus::haveDeviceWithRomID(OneWireAddress addr) {
        if (deviceCount == 0 || devices == nullptr) return false;
        for (int i=0;i<deviceCount;i++) {
            // cast the romid to uint64_t for fast and easy compare
            if (devices[i]->romid.id == addr.id) return true;
        }
        return false;
    }

    std::string OneWireTempBus::getAllDevices(bool printTemp, bool onlyNewDevices) {
        //byte i = 0;
        uint8_t done = 0;
        OneWireAddress addr;
        std::string returnStr;
        char hexString[3];
        bool first = true;

        returnStr.append("{\"pin\":");
        returnStr += std::to_string(pin);
        returnStr.append(",\"items\":[");
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
                    returnStr.append(",");
                }
                if (printTemp) {
                    returnStr.append("{\"romId\":");
                }
                returnStr.append("\"");
                for(int i = 0; i < 7; i++) 
                {
                    sprintf(hexString, "%02X", addr.bytes[i]);
                    returnStr.append(hexString);
                    returnStr.append(":");
                }
                sprintf(hexString, "%02X", addr.bytes[7]);
                returnStr.append(hexString);
                returnStr.append("\"");
                if (printTemp) {
                    returnStr.append(",\"tempC\":");
                    returnStr += std::to_string(dTemp->getTempC(addr.bytes));
                    returnStr.append(",\"tempF\":");
                    returnStr += std::to_string(dTemp->getTempF(addr.bytes));
                    returnStr.append("}");
                }
                first = false;
            }
        }
        
        returnStr.append("]}");
        return returnStr;
    }

    String OneWireTempBus::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\"";
        ret += DeviceConstStrings::pin;
        ret += std::to_string(pin).c_str();
        ret += ",\"devices\":[";
        for (int i=0;i<deviceCount;i++) {
            ret += "{";
            ret += devices[i]->ToString();
            ret += "}"; 
            if (i<deviceCount-1) ret += ",";
        }
        ret += "]";
        return ret;
    }

    OneWireTempBusAtRoot::OneWireTempBusAtRoot(const JsonVariant &jsonObj, const char* type) 
    : OneWireTempBus(jsonObj, type),
        autoRefresh(
              [this]() { requestTemperatures(); },
              [this]() { readAll(); },
              ParseRefreshTimeMs(jsonObj,HAL_JSON_ONE_WIRE_TEMP_DEFAULT_REFRESHRATE_MS)
        )
    {
        
    }

    OneWireTempBusAtRoot::~OneWireTempBusAtRoot() {
        
    }

    void OneWireTempBusAtRoot::loop() {
        autoRefresh.loop();
    }

    String OneWireTempBusAtRoot::ToString() {
        String ret;
        //ret += DeviceConstStrings::uid;
        //ret += decodeUID(uid).c_str();
        //ret += "\",";
        ret += DeviceConstStrings::type;
        ret += type;
        ret += "\",";
        ret += OneWireTempBus::ToString();
        ret += autoRefresh.ToString();
        
        return ret;
    }
}