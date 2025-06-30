

#include "HAL_JSON_DeviceTypesRegistry.h"

// all HAL devices to use here
#include "DeviceDrivers/HAL_JSON_CoreDevices.h"
#include "DeviceDrivers/OneWireTemp/HAL_JSON_OneWireTemp.h"
#include "DeviceDrivers/HAL_JSON_DHT.h"

namespace HAL_JSON {
    const char* test = "hello";
    //char* test2 = "world"; // forbidden will generate warning

    const DeviceTypeDef DeviceRegistry[] = {

        {UseRootUID::Mandatory, "DIN", DigitalInput::Create, DigitalInput::VerifyJSON},
        {UseRootUID::Mandatory, "DOUT", DigitalOutput::Create, DigitalOutput::VerifyJSON},
        {UseRootUID::Mandatory, "DPOUT", SinglePulseOutput::Create, SinglePulseOutput::VerifyJSON},
        {UseRootUID::Mandatory, "ADC", AnalogInput::Create, AnalogInput::VerifyJSON},
        {UseRootUID::Optional, "PWM_AW", PWMAnalogWrite::Create, PWMAnalogWrite::VerifyJSON},
        {UseRootUID::Mandatory, "PWM_AW_CFG", PWMAnalogWriteConfig::Create, PWMAnalogWriteConfig::VerifyJSON},

        {UseRootUID::Optional, "1WTG", OneWireTempGroup::Create, OneWireTempGroup::VerifyJSON},
        {UseRootUID::Optional, "1WTB", OneWireTempBusAtRoot::Create, OneWireTempBus::VerifyJSON},
        {UseRootUID::Mandatory, "1WTD", OneWireTempDeviceAtRoot::Create, OneWireTempDeviceAtRoot::VerifyJSON},

        {UseRootUID::Mandatory, "DHT", DHT::Create, DHT::VerifyJSON}, // under development
        {UseRootUID::Mandatory, "TX433", nullptr, nullptr},
        {UseRootUID::Mandatory, "REGO600", nullptr, nullptr},
        {UseRootUID::Mandatory, "PWM_LEDC", nullptr, nullptr},

        {UseRootUID::Void, nullptr, nullptr, nullptr} // terminator
    };
}