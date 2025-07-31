

#include "HAL_JSON_DeviceTypesRegistry.h"

// all HAL devices to use here
#include "Devices/HAL_JSON_RuleVariable.h"
#include "Devices/HAL_JSON_CoreDevices.h"
#include "Devices/OneWireTemp/HAL_JSON_OneWireTemp.h"
#include "Devices/HAL_JSON_DHT.h"
#include "Devices/RF433/HAL_JSON_TX433.h"
#include "Devices/REGO600/HAL_JSON_REGO600.h"

namespace HAL_JSON {
    const char* test = "hello";
    //char* test2 = "world"; // forbidden will generate warning

    const DeviceTypeDef DeviceRegistry[] = {
        {UseRootUID::Mandatory, "VAR", RuleVariable::Create, RuleVariable::VerifyJSON},

        {UseRootUID::Mandatory, "DIN", DigitalInput::Create, DigitalInput::VerifyJSON},
        {UseRootUID::Mandatory, "DOUT", DigitalOutput::Create, DigitalOutput::VerifyJSON},
        {UseRootUID::Mandatory, "DPOUT", SinglePulseOutput::Create, SinglePulseOutput::VerifyJSON},
#if defined(ESP32) || defined(_WIN32)
        {UseRootUID::Mandatory, "ADC", AnalogInput::Create, AnalogInput::VerifyJSON},
#endif
        {UseRootUID::Optional, "PWM_AW", PWMAnalogWrite::Create, PWMAnalogWrite::VerifyJSON},
        {UseRootUID::Mandatory, "PWM_AW_CFG", PWMAnalogWriteConfig::Create, PWMAnalogWriteConfig::VerifyJSON},

        {UseRootUID::Optional, "1WTG", OneWireTempGroup::Create, OneWireTempGroup::VerifyJSON},
        {UseRootUID::Optional, "1WTB", OneWireTempBusAtRoot::Create, OneWireTempBus::VerifyJSON},
        {UseRootUID::Mandatory, "1WTD", OneWireTempDeviceAtRoot::Create, OneWireTempDeviceAtRoot::VerifyJSON},

        {UseRootUID::Mandatory, "DHT", DHT::Create, DHT::VerifyJSON},
        {UseRootUID::Mandatory, "TX433", TX433::Create, TX433::VerifyJSON},
        {UseRootUID::Mandatory, "REGO600", REGO600::Create, REGO600::VerifyJSON},
#if defined(ESP32) || defined(_WIN32)
        {UseRootUID::Mandatory, "PWM_LEDC", nullptr, nullptr},
#endif

        {UseRootUID::Void, nullptr, nullptr, nullptr} // terminator
    };
}