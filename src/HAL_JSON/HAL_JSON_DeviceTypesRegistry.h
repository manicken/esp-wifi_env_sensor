
#include <Arduino.h>
#include <ArduinoJson.h>
#include "HAL_JSON_Value.h"
#include "HAL_JSON_Device.h"
// all HAL devices here
#include "HAL_JSON_DeviceTypeDefNames.h"
#include "HAL_JSON_CoreDevices.h"
#include "HAL_JSON_OneWireTemp.h"

namespace HAL_JSON {
    typedef Device* (*HAL_DEVICE_CREATE_FUNC)(JsonVariant &json);
    typedef HAL_JSON_VERIFY_JSON_RETURN_TYPE (*HAL_DEVICE_VERIFY_JSON_FUNC)(JsonVariant &json);

    typedef struct DeviceTypeDef {
        const char* type;
        HAL_DEVICE_CREATE_FUNC Create_Function;
        HAL_DEVICE_VERIFY_JSON_FUNC Verify_JSON_Function;
    } DeviceTypeDef ;

    DeviceTypeDef DeviceRegistry[] = {
        {HAL_JSON_TYPE_DIN, DigitalInput::Create, DigitalInput::VerifyJSON},
        {HAL_JSON_TYPE_DOUT, DigitalOutput::Create, DigitalOutput::VerifyJSON},
        {HAL_JSON_TYPE_DPOUT, SinglePulseOutput::Create, SinglePulseOutput::VerifyJSON},
        {HAL_JSON_TYPE_ADC, AnalogInput::Create, AnalogInput::VerifyJSON},
        {HAL_JSON_TYPE_PWM_ANALOG_WRITE, AnalogInput::Create, AnalogInput::VerifyJSON},
        {HAL_JSON_TYPE_ONE_WIRE_TEMP_GROUP, OneWireTempGroup::Create, OneWireTempGroup::VerifyJSON},
        {HAL_JSON_TYPE_ONE_WIRE_TEMP_BUS, OneWireTempGroup::Create, OneWireTempGroup::VerifyJSON},
        {HAL_JSON_TYPE_ONE_WIRE_TEMP_DEVICE, OneWireTempGroup::Create, OneWireTempGroup::VerifyJSON},
        {HAL_JSON_TYPE_REGO600, nullptr, nullptr}, // TODO implement
        {HAL_JSON_TYPE_PWM_LEDC, nullptr, nullptr}, // TODO implement
        {HAL_JSON_TYPE_DHT, nullptr, nullptr}, // TODO implement
        {HAL_JSON_TYPE_TX433, nullptr, nullptr}, // TODO implement

        {nullptr, nullptr, nullptr} // terminator 
    };
}