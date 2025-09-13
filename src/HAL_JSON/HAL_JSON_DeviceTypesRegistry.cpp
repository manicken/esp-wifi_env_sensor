

#include "HAL_JSON_DeviceTypesRegistry.h"

// all HAL devices to use here
#include "Devices/HAL_JSON_ScriptVariable.h"
#include "Devices/HAL_JSON_ScriptVariableReadOnly.h"
#include "Devices/HAL_JSON_ScriptVariableWriteOnlyTest.h"
#include "Devices/HAL_JSON_CoreDevices.h"
#include "Devices/OneWireTemp/HAL_JSON_OneWireTemp.h"
#include "Devices/HAL_JSON_DHT.h"
#include "Devices/RF433/HAL_JSON_TX433.h"
#include "Devices/REGO600/HAL_JSON_REGO600.h"

namespace HAL_JSON {

    const DeviceTypeDef DeviceRegistry[] = {
        {UseRootUID::Mandatory, "VAR", ScriptVariable::Create, ScriptVariable::VerifyJSON},


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
        {UseRootUID::Mandatory, "CONSTVAR", ScriptVariableReadOnly::Create, ScriptVariableReadOnly::VerifyJSON},
        {UseRootUID::Mandatory, "WRITEVAR", ScriptVariableWriteOnlyTest::Create, ScriptVariableWriteOnlyTest::VerifyJSON},

        /** mandatory null terminator */
        {UseRootUID::Void, nullptr, nullptr, nullptr} // terminator
    };

    const DeviceTypeDef* GetDeviceTypeDef(const char* type) {
        int i=0;
        while (true) {
            const DeviceTypeDef& def = DeviceRegistry[i++];
            if (def.typeName == nullptr) break;
            if (strcasecmp(def.typeName, type) == 0) return &def;
        }
        return nullptr;
    }
}