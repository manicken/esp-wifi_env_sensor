
#include "HAL_JSON_OneWireTempDevice.h"

namespace HAL_JSON {
    Device* OneWireTempDeviceAtRoot::Create(const JsonVariant& jsonObj) {
        return new OneWireTempDeviceAtRoot(jsonObj);
    }

    bool OneWireTempDevice::VerifyJSON(const JsonVariant &jsonObj) {
        if (!ValidateJsonStringField(jsonObj, HAL_JSON_KEYNAME_UID)) return false;
        if (!ValidateJsonStringField(jsonObj, HAL_JSON_KEYNAME_ONE_WIRE_ROMID)) return false;
        
        const char* romIdStr = jsonObj[HAL_JSON_KEYNAME_ONE_WIRE_ROMID].as<const char*>();
        if (strlen(romIdStr) == 0) { GlobalLogger.Error(F("OneWireTempDevice romId is zero lenght")); return false; }
        return Convert::HexToBytes(romIdStr, nullptr, 8);
    }

    OneWireTempDevice::OneWireTempDevice(const JsonVariant &jsonObj) : Device(UIDPathMaxLength::One) {
        const char* uidStr = jsonObj[HAL_JSON_KEYNAME_UID].as<const char*>();
        uid = encodeUID(uidStr);
        const char* romIdStr = jsonObj[HAL_JSON_KEYNAME_ONE_WIRE_ROMID].as<const char*>();
        Convert::HexToBytes(romIdStr, romid, 8);
        // optional settings
        if (ValidateJsonStringField(jsonObj, HAL_JSON_KEYNAME_ONE_WIRE_TEMPFORMAT)) {
            const char* tempFormatStr = jsonObj[HAL_JSON_KEYNAME_ONE_WIRE_TEMPFORMAT].as<const char*>();
            if (tempFormatStr[0] == 'c' || tempFormatStr[0] == 'C')
                format = OneWireTempDeviceTempFormat::Celsius;
            else if (tempFormatStr[0] == 'f' || tempFormatStr[0] == 'F')
                format = OneWireTempDeviceTempFormat::Fahrenheit;
            // else the default value is used (defined in .h file)
        }
    }

    OneWireTempDevice::~OneWireTempDevice() {
        
    }

    bool OneWireTempDevice::read(HALValue& val) {
        val = value;
        return true;
    }

    bool OneWireTempDeviceAtRoot::VerifyJSON(const JsonVariant &jsonObj) {
        if (OneWireTempDevice::VerifyJSON(jsonObj) == false) return false;
        uint8_t pin = jsonObj[HAL_JSON_KEYNAME_PIN].as<uint8_t>(); 
        return GPIO_manager::CheckIfPinAvailableAndReserve(pin, (static_cast<uint8_t>(GPIO_manager::PinMode::OUT) | static_cast<uint8_t>(GPIO_manager::PinMode::IN)));
    }

    OneWireTempDeviceAtRoot::OneWireTempDeviceAtRoot(const JsonVariant &jsonObj) 
        : OneWireTempDevice(jsonObj), 
          autoRefresh(
            [this](){ requestTemperatures(); },
            [this](){ readAll(); }
        )
    {
        autoRefresh.SetRefreshTimeMs(OneWireTempAutoRefresh::ParseRefreshTimeMs(jsonObj));
        pin = jsonObj[HAL_JSON_KEYNAME_PIN].as<uint8_t>();
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
            value = dTemp->getTempC(romid);
        else if (format == OneWireTempDeviceTempFormat::Fahrenheit)
            value = dTemp->getTempF(romid);
    }

    void OneWireTempDeviceAtRoot::loop() {
        autoRefresh.loop();
    }

}