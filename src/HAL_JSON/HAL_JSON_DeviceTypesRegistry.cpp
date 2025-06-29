

#include "HAL_JSON_DeviceTypesRegistry.h"

namespace HAL_JSON {

    const DeviceTypeDef DeviceRegistry[] = {
        {UseRootUID::Mandatory, HAL_JSON_TYPE_DIN, DigitalInput::Create, DigitalInput::VerifyJSON},
        {UseRootUID::Mandatory, HAL_JSON_TYPE_DOUT, DigitalOutput::Create, DigitalOutput::VerifyJSON},
        {UseRootUID::Mandatory, HAL_JSON_TYPE_DPOUT, SinglePulseOutput::Create, SinglePulseOutput::VerifyJSON},
        {UseRootUID::Mandatory, HAL_JSON_TYPE_ADC, AnalogInput::Create, AnalogInput::VerifyJSON},
        {UseRootUID::Mandatory, HAL_JSON_TYPE_PWM_ANALOG_WRITE, AnalogInput::Create, AnalogInput::VerifyJSON},

        {UseRootUID::Optional, HAL_JSON_TYPE_ONE_WIRE_TEMP_GROUP, OneWireTempGroup::Create, OneWireTempGroup::VerifyJSON},
        {UseRootUID::Optional, HAL_JSON_TYPE_ONE_WIRE_TEMP_BUS, OneWireTempBusAtRoot::Create, OneWireTempBus::VerifyJSON},
        {UseRootUID::Mandatory, HAL_JSON_TYPE_ONE_WIRE_TEMP_DEVICE, OneWireTempDeviceAtRoot::Create, OneWireTempDeviceAtRoot::VerifyJSON},

        {UseRootUID::Mandatory, HAL_JSON_TYPE_DHT, nullptr, nullptr},
        {UseRootUID::Mandatory, HAL_JSON_TYPE_TX433, nullptr, nullptr},
        {UseRootUID::Mandatory, HAL_JSON_TYPE_REGO600, nullptr, nullptr},
        {UseRootUID::Mandatory, HAL_JSON_TYPE_PWM_LEDC, nullptr, nullptr},

        {UseRootUID::Void, nullptr, nullptr, nullptr} // terminator
    };
}