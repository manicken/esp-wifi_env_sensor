
#include "HAL_JSON_OneWireTemp.h"

namespace HAL_JSON {
    namespace OneWireTemp {

        bool VerifyJSON(const JsonVariant &jsonObj) {
            // the type don't need any failsafe check as that is taken care of outside this call and is allways available
            const char* typeStr = jsonObj[HAL_JSON_KEYNAME_TYPE].as<const char*>();
            // ***************** GROUP ******************
            if (strcmp(typeStr, HAL_JSON_TYPE_ONE_WIRE_TEMP_GROUP) == 0)
            {
                return OneWireTempGroup::VerifyJSON(jsonObj);
            }
            // *****************  BUS  ********************
            else if (strcmp(typeStr, HAL_JSON_TYPE_ONE_WIRE_TEMP_BUS) == 0)
            {
                return OneWireTempBus::VerifyJSON(jsonObj);
            }
            // **************** DEVICE **********************
            else if (strcmp(typeStr, HAL_JSON_TYPE_ONE_WIRE_TEMP_DEVICE) == 0)
            {
                if (jsonObj.containsKey(HAL_JSON_KEYNAME_PIN) == false) {
                    GlobalLogger.Error(HAL_JSON_ERR_MISSING_KEY(HAL_JSON_KEYNAME_PIN));
                    return false;
                }
                return OneWireTempDevice::VerifyJSON(jsonObj);
            }
            GlobalLogger.Error(F("DeviceTypesRegistryError"));
            return false;
        }

        double ParseRefreshTime(const JsonVariant &value) {
            if (value.is<float>() || value.is<double>()) {
                return value.as<double>();
            } else if (value.is<const char*>()) {
                return atof(value.as<const char*>());
            } else if (value.is<uint32_t>()) {
                return static_cast<double>(value.as<uint32_t>());
            }
            return -1.0; // Invalid
        }

        uint32_t ParseRefreshTimeMs(const JsonVariant &jsonObj) {
            double rawSec = 1.0;
            if (jsonObj.containsKey(HAL_JSON_KEYNAME_REFRESHTIME_SEC)) {
                double val = ParseRefreshTime(jsonObj[HAL_JSON_KEYNAME_REFRESHTIME_SEC]);
                if (val >= 1.0) rawSec = val;
            }
            else if (jsonObj.containsKey(HAL_JSON_KEYNAME_REFRESHTIME_MIN)) {
                double val = ParseRefreshTime(jsonObj[HAL_JSON_KEYNAME_REFRESHTIME_MIN]);
                val *= 60.0;
                if (val >= 1.0) rawSec = val;
            }
            else {
                GlobalLogger.Warn(F("refreshrate is not set default is:"),String(HAL_JSON_ONE_WIRE_TEMP_DEFAULT_REFRESHRATE_MS).c_str());
            }
            return static_cast<uint32_t>(round(rawSec * 1000));
        }

        Device* Create(const JsonVariant &jsonObj) {
            return new OneWireTempGroup(jsonObj);
        }
    }
    
    //   ██████  ██████   ██████  ██    ██ ██████  
    //  ██       ██   ██ ██    ██ ██    ██ ██   ██ 
    //  ██   ███ ██████  ██    ██ ██    ██ ██████  
    //  ██    ██ ██   ██ ██    ██ ██    ██ ██      
    //   ██████  ██   ██  ██████   ██████  ██      
    
    bool OneWireTempGroup::VerifyJSON(const JsonVariant &jsonObj) {
        if (jsonObj.containsKey(HAL_JSON_KEYNAME_ITEMS) == false) {
            GlobalLogger.Error(HAL_JSON_ERR_MISSING_KEY(HAL_JSON_KEYNAME_ITEMS));
            return false;
        }
        if (jsonObj[HAL_JSON_KEYNAME_ITEMS].is<JsonArray>() == false) {
            GlobalLogger.Error(HAL_JSON_ERR_VALUE_TYPE(HAL_JSON_KEYNAME_ITEMS " not array"));
            return false;
        }
        const JsonArray items = jsonObj[HAL_JSON_KEYNAME_ITEMS].as<JsonArray>();
        if (items.size() == 0) {
            GlobalLogger.Error(HAL_JSON_ERR_ITEMS_EMPTY("OneWireTempGroup"));
            return false;
        }
        size_t itemCount = items.size();
        size_t validItemCount = 0;
        for (int i=0;i<itemCount;i++) {
            const JsonVariant item = items[i];
            if (item.is<const char*>() == false) continue; // comment item
            if (OneWireTempBus::VerifyJSON(item) == true) validItemCount++;
        }
        if (validItemCount == 0) {
            GlobalLogger.Error(HAL_JSON_ERR_ITEMS_NOT_VALID("OneWireTempGroup"));
            return false;
        } 
        return true;
    }

    OneWireTempGroup::OneWireTempGroup(const JsonVariant &jsonObj) {
        refreshTimeMs = OneWireTemp::ParseRefreshTimeMs(jsonObj);

        // checked beforehand so extracting it here is safe
        const char* uidStr = jsonObj[HAL_JSON_KEYNAME_UID].as<const char*>();
        uid = encodeUID(uidStr);      

        // checked beforehand so extracting it here is safe
        const char* typeStr = jsonObj[HAL_JSON_KEYNAME_TYPE].as<const char*>();
        
        // ***************** GROUP ******************
        if (strcmp(typeStr, HAL_JSON_TYPE_ONE_WIRE_TEMP_GROUP) == 0)
        {
            type = OneWireTemp::Type::GROUP;
            busCount = 0;
            const JsonArray& items = jsonObj[HAL_JSON_KEYNAME_ITEMS].as<JsonArray>();
            uint32_t itemCount = items.size();
            // first pass count buscount
            for (int i=0;i<itemCount;i++) {
                if (OneWireTempBus::VerifyJSON(items[i]) == false) continue;
                busCount++;
            }
            busses = new OneWireTempBus*[busCount];
            // second pass create busses
            uint32_t index = 0;
            for (int i=0;i<itemCount;i++) {
                const JsonVariant& item = items[i];
                if (OneWireTempBus::VerifyJSON(item) == false) continue;
                busses[index++] = new OneWireTempBus(item, type);
            }
        }
        // *****************  BUS  ********************
        else if (strcmp(typeStr, HAL_JSON_TYPE_ONE_WIRE_TEMP_BUS) == 0)
        {
            type = OneWireTemp::Type::BUS;
            // there is only one bus 
            busCount = 1;
            busses = new OneWireTempBus*[1];
            busses[0] = new OneWireTempBus(jsonObj, type);
        }
        // **************** DEVICE **********************
        else if (strcmp(typeStr, HAL_JSON_TYPE_ONE_WIRE_TEMP_DEVICE) == 0)
        {
            type = OneWireTemp::Type::DEVICE;
            // allways create one default bus even if there is only one device
            // this is to avoid creating duplicate loop state machine code for each devicetype
            busCount = 1;
            busses = new OneWireTempBus*[1];
            busses[0] = new OneWireTempBus(jsonObj, type);
        }
    }
    OneWireTempGroup::~OneWireTempGroup() {
        if (busses != nullptr) {
            for (int i=0;i<busCount;i++)
                delete busses[i];
        }
        delete[] busses;
        busses = nullptr;
    }

    bool OneWireTempGroup::read(const HALReadRequest &req) {
        return false;
    }
    bool OneWireTempGroup::write(const HALWriteRequest&req) {
        return false;
    }

    void OneWireTempGroup::loop() {
        uint32_t now = millis();
        switch (state) {
            case State::IDLE:
                if (now - lastUpdateMs >= refreshTimeMs) {
                    for (int i=0;i<busCount;i++) {
                        busses[i]->requestTemperatures();
                    }
                    state = State::WAITING_FOR_CONVERSION;
                    lastStart = now;
                }
                break;
            case State::WAITING_FOR_CONVERSION:
                if (now - lastStart >= refreshTimeMs) {
                    for (int i=0;i<busCount;i++) {
                        busses[i]->readAll();
                    }
                    lastUpdateMs = now;
                    state = State::IDLE;
                }
                break;
        }
    }

    //  ██████  ██    ██ ███████ 
    //  ██   ██ ██    ██ ██      
    //  ██████  ██    ██ ███████ 
    //  ██   ██ ██    ██      ██ 
    //  ██████   ██████  ███████ 

    bool OneWireTempBus::VerifyJSON(const JsonVariant &jsonObj) {
        if (jsonObj.containsKey(HAL_JSON_KEYNAME_PIN) == false) { GlobalLogger.Error(HAL_JSON_ERR_MISSING_KEY(HAL_JSON_KEYNAME_PIN)); return false; }
        if (jsonObj[HAL_JSON_KEYNAME_PIN].is<uint8_t>() == false)  { GlobalLogger.Error(HAL_JSON_ERR_VALUE_TYPE(HAL_JSON_KEYNAME_PIN)); return false; }
        uint8_t pin = jsonObj[HAL_JSON_KEYNAME_PIN].as<uint8_t>(); 
        if (GPIO_manager::CheckIfPinAvailableAndReserve(pin, (static_cast<uint8_t>(GPIO_manager::PinMode::OUT) | static_cast<uint8_t>(GPIO_manager::PinMode::IN))) == false) {
            return false;
        }
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
            if (OneWireTempDevice::VerifyJSON(item) == true) validItemCount++;
        }
        if (validItemCount == 0) { GlobalLogger.Error(HAL_JSON_ERR_ITEMS_NOT_VALID("OneWireTempBus")); return false; }
        return true;
    }

    OneWireTempBus::OneWireTempBus(const JsonVariant &jsonObj, OneWireTemp::Type type) {
        pin = jsonObj[HAL_JSON_KEYNAME_PIN].as<uint8_t>();
        // pin is reserved in ValidateJSON

        oneWire = new OneWire(pin);
        dTemp = new DallasTemperature(oneWire);
        dTemp->setWaitForConversion(false);

        if (type == OneWireTemp::Type::DEVICE) // special case
        {
            deviceCount = 1;
            devices = new OneWireTempDevice*[1];
            devices[0] = new OneWireTempDevice(jsonObj);
        }
        else
        {
            deviceCount = 0;
            JsonArray items = jsonObj[HAL_JSON_KEYNAME_ITEMS].as<JsonArray>();
            uint32_t itemCount = items.size();
            // first pass count valid devices
            for (int i=0;i<itemCount;i++) {
                JsonVariant item = items[i];
                if (OneWireTempDevice::VerifyJSON(item) == false) continue;
                deviceCount++;
            }
            devices = new OneWireTempDevice*[deviceCount];
            uint32_t index = 0;
            for (int i=0;i<itemCount;i++) {
                JsonVariant item = items[i];
                if (OneWireTempDevice::VerifyJSON(item) == false) continue;
                devices[index++] = new OneWireTempDevice(item);
            }
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
            if (devices[i]->format == OneWireTemp::TempFormat::Celsius)
                devices[i]->value = dTemp->getTempC(devices[i]->romid);
            else if (devices[i]->format == OneWireTemp::TempFormat::Fahrenheit)
                devices[i]->value = dTemp->getTempF(devices[i]->romid);
        }
    }

    //  ██████  ███████ ██    ██ ██  ██████ ███████ 
    //  ██   ██ ██      ██    ██ ██ ██      ██      
    //  ██   ██ █████   ██    ██ ██ ██      █████   
    //  ██   ██ ██       ██  ██  ██ ██      ██      
    //  ██████  ███████   ████   ██  ██████ ███████ 

    bool OneWireTempDevice::VerifyJSON(const JsonVariant &jsonObj) {
        if (!ValidateJsonStringField(jsonObj, HAL_JSON_KEYNAME_UID)) return false;
        if (!ValidateJsonStringField(jsonObj, HAL_JSON_KEYNAME_ONE_WIRE_ROMID)) return false;
        
        const char* romIdStr = jsonObj[HAL_JSON_KEYNAME_ONE_WIRE_ROMID].as<const char*>();
        if (strlen(romIdStr) == 0) { GlobalLogger.Error(F("OneWireTempDevice romId is zero lenght")); return false; }
        return Convert::HexToBytes(romIdStr, nullptr, 8);
    }

    OneWireTempDevice::OneWireTempDevice(const JsonVariant &jsonObj) {
        const char* uidStr = jsonObj[HAL_JSON_KEYNAME_UID].as<const char*>();
        uid = encodeUID(uidStr);
        const char* romIdStr = jsonObj[HAL_JSON_KEYNAME_ONE_WIRE_ROMID].as<const char*>();
        Convert::HexToBytes(romIdStr, romid, 8);
        // optional settings
        if (ValidateJsonStringField(jsonObj, HAL_JSON_KEYNAME_ONE_WIRE_TEMPFORMAT)) {
            const char* tempFormatStr = jsonObj[HAL_JSON_KEYNAME_ONE_WIRE_TEMPFORMAT].as<const char*>();
            if (tempFormatStr[0] == 'c' || tempFormatStr[0] == 'C')
                format = OneWireTemp::TempFormat::Celsius;
            else if (tempFormatStr[0] == 'f' || tempFormatStr[0] == 'F')
                format = OneWireTemp::TempFormat::Fahrenheit;
            // else the default value is used (defined in .h file)
        }
    }

    OneWireTempDevice::~OneWireTempDevice() {
        
    }
}