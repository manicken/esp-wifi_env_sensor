
#include "HAL_JSON_OneWireTempDevice.h"

namespace HAL_JSON {
    


    //   ██████  ███    ██ ███████     ██     ██ ██ ██████  ███████     ████████ ███████ ███    ███ ██████      ██████  ███████ ██    ██ ██  ██████ ███████ 
    //  ██    ██ ████   ██ ██          ██     ██ ██ ██   ██ ██             ██    ██      ████  ████ ██   ██     ██   ██ ██      ██    ██ ██ ██      ██      
    //  ██    ██ ██ ██  ██ █████       ██  █  ██ ██ ██████  █████          ██    █████   ██ ████ ██ ██████      ██   ██ █████   ██    ██ ██ ██      █████   
    //  ██    ██ ██  ██ ██ ██          ██ ███ ██ ██ ██   ██ ██             ██    ██      ██  ██  ██ ██          ██   ██ ██       ██  ██  ██ ██      ██      
    //   ██████  ██   ████ ███████      ███ ███  ██ ██   ██ ███████        ██    ███████ ██      ██ ██          ██████  ███████   ████   ██  ██████ ███████ 
    //                                                                                                                                                     

    bool OneWireTempDevice::VerifyJSON(const JsonVariant &jsonObj) {
        
        if (!ValidateJsonStringField(jsonObj, HAL_JSON_KEYNAME_UID)){ SET_ERR_LOC(HAL_JSON_ERROR_SOURCE_1WTD_VERIFY_JSON); return false; }
        if (!ValidateJsonStringField(jsonObj, HAL_JSON_KEYNAME_ONE_WIRE_ROMID)){ SET_ERR_LOC(HAL_JSON_ERROR_SOURCE_1WTD_VERIFY_JSON); return false; }
        
        const char* romIdStr = GetAsConstChar(jsonObj,HAL_JSON_KEYNAME_ONE_WIRE_ROMID);//].as<const char*>();
        if (strlen(romIdStr) == 0) { GlobalLogger.Error(F("OneWireTempDevice romId is zero lenght")); return false; }
        return Convert::HexToBytes(romIdStr, nullptr, 8);
    }

    OneWireTempDevice::OneWireTempDevice(const JsonVariant &jsonObj, const char* type) : Device(UIDPathMaxLength::One, type) {
        const char* uidStr = GetAsConstChar(jsonObj,HAL_JSON_KEYNAME_UID);//].as<const char*>();
        uid = encodeUID(uidStr);
        const char* romIdStr = GetAsConstChar(jsonObj,HAL_JSON_KEYNAME_ONE_WIRE_ROMID);//].as<const char*>();
        Convert::HexToBytes(romIdStr, romid.bytes, 8);
        // optional settings
        if (jsonObj.containsKey(HAL_JSON_KEYNAME_ONE_WIRE_TEMPFORMAT) && ValidateJsonStringField_noContains(jsonObj, HAL_JSON_KEYNAME_ONE_WIRE_TEMPFORMAT)) {
            const char* tempFormatStr = GetAsConstChar(jsonObj, HAL_JSON_KEYNAME_ONE_WIRE_TEMPFORMAT);//].as<const char*>();
            if (tempFormatStr[0] == 'c' || tempFormatStr[0] == 'C')
                format = OneWireTempDeviceTempFormat::Celsius;
            else if (tempFormatStr[0] == 'f' || tempFormatStr[0] == 'F')
                format = OneWireTempDeviceTempFormat::Fahrenheit;
            // else the default value is used (defined in .h file)
        }
    }

    OneWireTempDevice::~OneWireTempDevice() {
        
    }

    HALOperationResult  OneWireTempDevice::read(HALValue& val) {
        val = value;
        return HALOperationResult::Success;
    }

    
    
    String OneWireTempDevice::ToString() {
        String ret;
        ret.reserve(128);
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",\"romid\":\"";
        ret += String(Convert::ByteArrayToString(romid.bytes, 8).c_str());
        ret += "\",\"format\":";
        if (format == OneWireTempDeviceTempFormat::Celsius) ret += "\"C\"";
        else if (format == OneWireTempDeviceTempFormat::Fahrenheit) ret += "\"F\"";
        else ret += "\"other\"";
        ret += ",";
        ret += DeviceConstStrings::value;//StartWithComma;
        ret += std::to_string(value).c_str();
        //ret += "\"";
        return ret;
    }

    //   ██     ██     ██ ██ ██████  ███████     ████████ ███████ ███    ███ ██████      ██████  ███████ ██    ██ ██  ██████ ███████      ██████      ██████   ██████   ██████  ████████ 
    //  ███     ██     ██ ██ ██   ██ ██             ██    ██      ████  ████ ██   ██     ██   ██ ██      ██    ██ ██ ██      ██          ██    ██     ██   ██ ██    ██ ██    ██    ██    
    //   ██     ██  █  ██ ██ ██████  █████          ██    █████   ██ ████ ██ ██████      ██   ██ █████   ██    ██ ██ ██      █████       ██ ██ ██     ██████  ██    ██ ██    ██    ██    
    //   ██     ██ ███ ██ ██ ██   ██ ██             ██    ██      ██  ██  ██ ██          ██   ██ ██       ██  ██  ██ ██      ██          ██ ██ ██     ██   ██ ██    ██ ██    ██    ██    
    //   ██      ███ ███  ██ ██   ██ ███████        ██    ███████ ██      ██ ██          ██████  ███████   ████   ██  ██████ ███████      █ ████      ██   ██  ██████   ██████     ██    

    Device* OneWireTempDeviceAtRoot::Create(const JsonVariant& jsonObj, const char* type) {
        return new OneWireTempDeviceAtRoot(jsonObj, type);
    }

    bool OneWireTempDeviceAtRoot::VerifyJSON(const JsonVariant &jsonObj) {
        if (OneWireTempDevice::VerifyJSON(jsonObj) == false) return false;
        return GPIO_manager::ValidateJsonAndCheckIfPinAvailableAndReserve(jsonObj, (static_cast<uint8_t>(GPIO_manager::PinMode::OUT) | static_cast<uint8_t>(GPIO_manager::PinMode::IN)));
    }

    OneWireTempDeviceAtRoot::OneWireTempDeviceAtRoot(const JsonVariant &jsonObj, const char* type) 
        : OneWireTempDevice(jsonObj, type), 
          autoRefresh(
            [this](){ requestTemperatures(); },
            [this](){ readAll(); },
            ParseRefreshTimeMs(jsonObj,HAL_JSON_ONE_WIRE_TEMP_DEFAULT_REFRESHRATE_MS)
        )
    {
        pin = GetAsUINT32(jsonObj,HAL_JSON_KEYNAME_PIN);//].as<uint8_t>();
        GPIO_manager::ReservePin(pin); // this is in most cases taken care of in OneWireTempBus::VerifyJSON but there are situations where it's needed

        oneWire = new OneWire(pin);
        dTemp = new DallasTemperature(oneWire);
        dTemp->setWaitForConversion(false);
    }

    OneWireTempDeviceAtRoot::~OneWireTempDeviceAtRoot() {
        
    }

    void OneWireTempDeviceAtRoot::requestTemperatures() {
        dTemp->requestTemperatures();
    }

    void OneWireTempDeviceAtRoot::readAll() {
        if (format == OneWireTempDeviceTempFormat::Celsius)
            value = dTemp->getTempC(romid.bytes);
        else if (format == OneWireTempDeviceTempFormat::Fahrenheit)
            value = dTemp->getTempF(romid.bytes);
    }

    HALOperationResult  OneWireTempDeviceAtRoot::write(const HALValue& val) {
#ifdef _WIN32
        dTemp->setTempC(val); // only in simulator
        return HALOperationResult::Success;
#endif
        return HALOperationResult::UnsupportedOperation;
    }

    void OneWireTempDeviceAtRoot::loop() {
        autoRefresh.loop();
    }

    String OneWireTempDeviceAtRoot::ToString() {
        String ret;
        ret += DeviceConstStrings::type;
        ret += type;
        ret += "\",";
        ret += OneWireTempDevice::ToString();
        ret += autoRefresh.ToString();
        return ret;
    }

}