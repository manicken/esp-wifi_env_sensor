
#include "HAL_JSON_I2C_BUS_DeviceTypeReg.h"

// Available I2C device types here
#include "Display_SSD1306/HAL_JSON_Display_SSD1306.h"
#include "HAL_JSON_PCF8574x.h"

namespace HAL_JSON {

    const I2C_DeviceTypeDef I2C_DeviceRegistry[] = {
        {"SSD1306", Display_SSD1306::Create, Display_SSD1306::VerifyJSON},
        {"PCF8574x",  PCF8574x::Create, PCF8574x::VerifyJSON},
        /** mandatory null terminator */
        {nullptr, nullptr, nullptr}
    };
    const I2C_DeviceTypeDef* GetI2C_DeviceTypeDef(const char* type) {
        int i=0;
        while (true) {
            const I2C_DeviceTypeDef& def = I2C_DeviceRegistry[i++];
            if (def.typeName == nullptr) break;
            if (strcasecmp(def.typeName, type) == 0) return &def;
        }
        return nullptr;
    }
}