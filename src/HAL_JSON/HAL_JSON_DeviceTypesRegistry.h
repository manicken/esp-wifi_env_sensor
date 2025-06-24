
#include <Arduino.h>
#include <ArduinoJson.h>
#include "HAL_JSON_Value.h"
#include "HAL_JSON_Device.h"
// all HAL devices here
#include "HAL_JSON_DeviceTypeDefNames.h"
#include "DeviceDrivers/HAL_JSON_CoreDevices.h"
#include "DeviceDrivers/HAL_JSON_OneWireTemp.h"

namespace HAL_JSON {
    typedef Device* (*HAL_DEVICE_CREATE_FUNC)(const JsonVariant &json);
    typedef bool (*HAL_DEVICE_VERIFY_JSON_FUNC)(const JsonVariant &json);

    enum class UseRootUID {
        Mandatory,
        Optional,
        Void
    };

    typedef struct DeviceTypeDef {
        UseRootUID useRootUID;
        const char* type;
        HAL_DEVICE_CREATE_FUNC Create_Function;
        HAL_DEVICE_VERIFY_JSON_FUNC Verify_JSON_Function;
    } DeviceTypeDef ;



    DeviceTypeDef DeviceRegistry[] = {
        {UseRootUID::Mandatory, HAL_JSON_TYPE_DIN, DigitalInput::Create, DigitalInput::VerifyJSON},
        {UseRootUID::Mandatory, HAL_JSON_TYPE_DOUT, DigitalOutput::Create, DigitalOutput::VerifyJSON},
        {UseRootUID::Mandatory, HAL_JSON_TYPE_DPOUT, SinglePulseOutput::Create, SinglePulseOutput::VerifyJSON},
        {UseRootUID::Mandatory, HAL_JSON_TYPE_ADC, AnalogInput::Create, AnalogInput::VerifyJSON},
        {UseRootUID::Mandatory, HAL_JSON_TYPE_PWM_ANALOG_WRITE, AnalogInput::Create, AnalogInput::VerifyJSON},
        {UseRootUID::Optional, HAL_JSON_TYPE_ONE_WIRE_TEMP_GROUP, OneWireTemp::Create, OneWireTemp::VerifyJSON}, // currently in development
        {UseRootUID::Optional, HAL_JSON_TYPE_ONE_WIRE_TEMP_BUS, OneWireTemp::Create, OneWireTemp::VerifyJSON}, // currently in development
        {UseRootUID::Mandatory, HAL_JSON_TYPE_ONE_WIRE_TEMP_DEVICE, OneWireTemp::Create, OneWireTemp::VerifyJSON}, // currently in development
        {UseRootUID::Mandatory, HAL_JSON_TYPE_DHT, nullptr, nullptr}, // TODO implement
        {UseRootUID::Mandatory, HAL_JSON_TYPE_TX433, nullptr, nullptr}, // TODO implement
        {UseRootUID::Mandatory, HAL_JSON_TYPE_REGO600, nullptr, nullptr}, // TODO implement
        {UseRootUID::Mandatory, HAL_JSON_TYPE_PWM_LEDC, nullptr, nullptr}, // TODO implement

        {UseRootUID::Void, nullptr, nullptr, nullptr} // terminator 
    };
}