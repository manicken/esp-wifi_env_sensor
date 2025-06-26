
#include <Arduino.h>
#include <ArduinoJson.h>
#include "HAL_JSON_Value.h"
#include "HAL_JSON_Device.h"
// all HAL devices here
#include "HAL_JSON_DeviceTypeDefNames.h"
#include "DeviceDrivers/HAL_JSON_CoreDevices.h"
#include "DeviceDrivers/OneWireTemp/HAL_JSON_OneWireTemp.h"

namespace HAL_JSON {
    typedef Device* (*HAL_DEVICE_CREATE_FUNC)(const JsonVariant &json);
    typedef bool (*HAL_DEVICE_VERIFY_JSON_FUNC)(const JsonVariant &json);

    enum class UseRootUID {
        Mandatory,
        Optional,
        Void
    };

    enum class UIDPathMaxLength {
        Zero = 0, // only used for terminator device
        One = 1,
        Two = 2,
        Three = 3,
        Four = 4 // not used at the moment
    };

    typedef struct DeviceTypeDef {
        UIDPathMaxLength uidPathMaxLength;
        UseRootUID useRootUID;
        const char* type;
        HAL_DEVICE_CREATE_FUNC Create_Function;
        HAL_DEVICE_VERIFY_JSON_FUNC Verify_JSON_Function;
    } DeviceTypeDef ;



    DeviceTypeDef DeviceRegistry[] = {
        {UIDPathMaxLength::One, UseRootUID::Mandatory, HAL_JSON_TYPE_DIN, DigitalInput::Create, DigitalInput::VerifyJSON},
        {UIDPathMaxLength::One, UseRootUID::Mandatory, HAL_JSON_TYPE_DOUT, DigitalOutput::Create, DigitalOutput::VerifyJSON},
        {UIDPathMaxLength::One, UseRootUID::Mandatory, HAL_JSON_TYPE_DPOUT, SinglePulseOutput::Create, SinglePulseOutput::VerifyJSON},
        {UIDPathMaxLength::One, UseRootUID::Mandatory, HAL_JSON_TYPE_ADC, AnalogInput::Create, AnalogInput::VerifyJSON},
        {UIDPathMaxLength::One, UseRootUID::Mandatory, HAL_JSON_TYPE_PWM_ANALOG_WRITE, AnalogInput::Create, AnalogInput::VerifyJSON},
        // following three is OneWireTemp i.e. DS18B20
        {UIDPathMaxLength::Three, UseRootUID::Optional, HAL_JSON_TYPE_ONE_WIRE_TEMP_GROUP, OneWireTempGroup::Create, OneWireTempGroup::VerifyJSON}, // currently in development
        {UIDPathMaxLength::Two, UseRootUID::Optional, HAL_JSON_TYPE_ONE_WIRE_TEMP_BUS, OneWireTempBusAtRoot::Create, OneWireTempBus::VerifyJSON}, // currently in development
        {UIDPathMaxLength::One, UseRootUID::Mandatory, HAL_JSON_TYPE_ONE_WIRE_TEMP_DEVICE, OneWireTempDeviceAtRoot::Create, OneWireTempDeviceAtRoot::VerifyJSON}, // currently in development

        {UIDPathMaxLength::One, UseRootUID::Mandatory, HAL_JSON_TYPE_DHT, nullptr, nullptr}, // TODO implement
        {UIDPathMaxLength::One, UseRootUID::Mandatory, HAL_JSON_TYPE_TX433, nullptr, nullptr}, // TODO implement
        {UIDPathMaxLength::One, UseRootUID::Mandatory, HAL_JSON_TYPE_REGO600, nullptr, nullptr}, // TODO implement
        {UIDPathMaxLength::One, UseRootUID::Mandatory, HAL_JSON_TYPE_PWM_LEDC, nullptr, nullptr}, // TODO implement

        {UIDPathMaxLength::Zero, UseRootUID::Void, nullptr, nullptr, nullptr} // terminator 
    };
}