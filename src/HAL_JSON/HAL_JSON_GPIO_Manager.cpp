#include "HAL_JSON_GPIO_Manager.h"

#define MAKE_PIN_MASK_4(a, b, c, d) (static_cast<uint8_t>(a) | static_cast<uint8_t>(b) | static_cast<uint8_t>(c) | static_cast<uint8_t>(d))
#define MAKE_PIN_MASK_3(a, b, c)    (static_cast<uint8_t>(a) | static_cast<uint8_t>(b) | static_cast<uint8_t>(c))
#define MAKE_PIN_MASK_2(a, b)       (static_cast<uint8_t>(a) | static_cast<uint8_t>(b))
#define MAKE_PIN_MASK_1(a)          (static_cast<uint8_t>(a))

namespace HAL_JSON {

    namespace GPIO_manager
    {
    #if defined(ESP8266)
        const gpio_pin available_gpio_list[] {
            {0, MAKE_PIN_MASK_3(PinMode::OUT, PinMode::HIGH2BOOT, PinMode::SpecialAtBoot)},  // (reserved for programming) only safe to use as a output
            {1, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::SpecialAtBoot)},  // TXD0 (reserved for programming/UART) only safe to use as a output
            {2, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::SpecialAtBoot)},  // TXD1 (reserved for debug) only safe to use as a output
            //{3, MAKE_PIN_MASK_2(PinMode::Reserved, PinMode::SpecialAtBoot)},  // RXD0 (reserved for programming/UART)
            {4, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::IN)},    // I2C SDA
            {5, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::IN)},    // I2C SCL
            //{6, MAKE_PIN_MASK_2(PinMode::Reserved, PinMode::SpecialAtBoot)},  // SDCLK (reserved for spi flash)
            //{7, MAKE_PIN_MASK_2(PinMode::Reserved, PinMode::SpecialAtBoot)},  // SDD0 (reserved for spi flash)
            //{8, MAKE_PIN_MASK_2(PinMode::Reserved, PinMode::SpecialAtBoot)},  // SDD1 (reserved for spi flash)
            //{9, MAKE_PIN_MASK_2(PinMode::Reserved, PinMode::SpecialAtBoot)},  // SDD2 (reserved for spi flash)
            //{10, MAKE_PIN_MASK_2(PinMode::Reserved, PinMode::SpecialAtBoot)}, // SDD3 (reserved for spi flash)
            //{11, MAKE_PIN_MASK_2(PinMode::Reserved, PinMode::SpecialAtBoot)}, // SDCMD (reserved for spi flash)
            {12, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::IN)}, // SPI MISO
            {13, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::IN)}, // SPI MOSI
            {14, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::IN)}, // SPI SCLK
            {15, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::IN)}, // SPI CS/TXD2
        };
    #elif defined(ESP32)
        const gpio_pin available_gpio_list[] {
            {0, MAKE_PIN_MASK_3(PinMode::OUT, PinMode::HIGH2BOOT, PinMode::SpecialAtBoot)}, // ADC2_1/TOUCH1 (reserved for programming, better to just keep it a output)
            {1, MAKE_PIN_MASK_3(PinMode::Reserved, PinMode::OUT, PinMode::SpecialAtBoot)}, // U0_TXD (reserved for programmer/debug)
            {2, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::LOW2BOOT)}, // ADC2_2/TOUCH2/SD_DATA0 (must be LOW during boot/is connected to onboard LED, could be a output function only pin)
            {3, MAKE_PIN_MASK_2(PinMode::Reserved, PinMode::SpecialAtBoot)}, // U0_RXD (reserved for programmer/debug cannot be shared directly)
            {4, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::IN)},  // ADC2_0/TOUCH0/SD_DATA1 (ADC2 cannot be used together with WiFi)
            {5, MAKE_PIN_MASK_3(PinMode::OUT, PinMode::HIGH2BOOT, PinMode::SpecialAtBoot)},  // VSPI_CS (must be HIGH during boot better to keep it a output only)
    #ifndef ESP32WROVER_E_IE        
            {6, MAKE_PIN_MASK_2(PinMode::Reserved, PinMode::SpecialAtBoot)},  // U1_CTS/SPI_CLK (reserved for flash)
            {7, MAKE_PIN_MASK_2(PinMode::Reserved, PinMode::SpecialAtBoot)},  // U2_RTS/SPI_MISO (reserved for flash)
            {8, MAKE_PIN_MASK_2(PinMode::Reserved, PinMode::SpecialAtBoot)},  // U2_CTS/SPI_MOSI (reserved for flash)
            {9, MAKE_PIN_MASK_2(PinMode::Reserved, PinMode::SpecialAtBoot)},  // U1_RXD/SPI_HD (reserved for flash)
            {10, MAKE_PIN_MASK_2(PinMode::Reserved, PinMode::SpecialAtBoot)}, // U1_TXD/SPI_WP (reserved for flash)
            {11, MAKE_PIN_MASK_2(PinMode::Reserved, PinMode::SpecialAtBoot)}, // U1_RTX/SPI_CS (reserved for flash)
    #endif
            {12, MAKE_PIN_MASK_3(PinMode::OUT, PinMode::LOW2BOOT, PinMode::SpecialAtBoot)}, // ADC2_5/TOUCH5/HSPI_MISO/SD_DATA2 (must be LOW during boot, could be a output function only pin)
            {13, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::IN)}, // ADC2_4/TOUCH4/HSPI_MOSI/SD_DATA3 (ADC2 cannot be used together with WiFi)
            {14, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::IN)}, // ADC2_6/TOUCH6/HSPI_CLK/SD_CLK (ADC2 cannot be used together with WiFi)
            {15, MAKE_PIN_MASK_3(PinMode::OUT, PinMode::HIGH2BOOT, PinMode::SpecialAtBoot)}, // ADC2_3/TOUCH3/HSPI_CS/SD_CMD (must be HIGH during boot, could be a output function only pin)
    #ifndef ESP32WROVER_E_IE
            {16, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::IN)}, // U2_RXD
            {17, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::IN)}, // U2_TXD
    #endif       
            {18, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::IN)}, // VSPI_CLK
            {19, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::IN)}, // VSPI_MISO
            {21, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::IN)}, // I2C_SDA
            {22, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::IN)}, // I2C_SCL
            {23, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::IN)}, // VSPI_MOSI
            {25, MAKE_PIN_MASK_3(PinMode::OUT, PinMode::IN, PinMode::AOUT)}, // ADC2_8/DAC1 (ADC2 cannot be used together with WiFi)
            {26, MAKE_PIN_MASK_3(PinMode::OUT, PinMode::IN, PinMode::AOUT)}, // ADC2_9/DAC2 (ADC2 cannot be used together with WiFi)
            {27, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::IN)}, // ADC2_7/TOUCH7 (ADC2 cannot be used together with WiFi)
            {32, MAKE_PIN_MASK_3(PinMode::OUT, PinMode::IN, PinMode::AIN)}, // ADC1_4/TOUCH9/XTAL32
            {33, MAKE_PIN_MASK_3(PinMode::OUT, PinMode::IN, PinMode::AIN)}, // ADC1_5/TOUCH8/XTAL32
            {34, MAKE_PIN_MASK_2(PinMode::IN, PinMode::AIN)}, // ADC1_6 (input only)
            {35, MAKE_PIN_MASK_2(PinMode::IN, PinMode::AIN)}, // ADC1_7 (input only)
            {36, MAKE_PIN_MASK_2(PinMode::IN, PinMode::AIN)}, // ADC1_0/SensVP (input only)
            {39, MAKE_PIN_MASK_2(PinMode::IN, PinMode::AIN)},  // ADC1_3/SensVN (input only)
            {0xFF, 0x00} // terminator item
            };
    #endif
        int available_gpio_list_lenght = -1; // not set yet
        void set_available_gpio_list_length() {
            int len = 0;
            while (available_gpio_list[len].pin != 0xFF) ++len;
            available_gpio_list_lenght = len;
        }

        const PinModeDef PinModeStrings[] = {
            {"Reserved", static_cast<uint8_t>(PinMode::Reserved)},
            {"SpecialAtBoot", static_cast<uint8_t>(PinMode::SpecialAtBoot)},
            {"LOW2BOOT", static_cast<uint8_t>(PinMode::LOW2BOOT)},
            {"HIGH2BOOT", static_cast<uint8_t>(PinMode::HIGH2BOOT)},
            {"OUT", static_cast<uint8_t>(PinMode::OUT)},
            {"IN", static_cast<uint8_t>(PinMode::IN)},
            {"AIN", static_cast<uint8_t>(PinMode::AIN)},
            {"AOUT", static_cast<uint8_t>(PinMode::AOUT)},
            {nullptr, 0} // terminator item
        };

        int PinModeStrings_length = -1;
        void set_PinModeStrings_length() {
            int len = 0;
            while (PinModeStrings[len].Name != nullptr) ++len;
            PinModeStrings_length = len;
        }
        
        uint8_t* reservedPins = nullptr;

        std::string describePinMode(uint8_t mask) {
            std::string result;
            int i = 0;
            while (true) {
                const PinModeDef& pinModeDef = PinModeStrings[i];
                if (pinModeDef.Name == nullptr) break;
                if (mask & pinModeDef.mode) {
                    if (!result.empty()) result += "|";
                    result += pinModeDef.Name;
                }
                i++;
            }
            return result.empty() ? "None" : result;
        }

        bool CheckIfPinAvailable(uint8_t pin, uint8_t pinMode) {
            int i=0;
            while (true) {
                const gpio_pin& pinDef = available_gpio_list[i];
                if (pinDef.pin == 0xFF) break;
                if (pinDef.pin == pin) {
                    if (reservedPins[i] == 1) {
                        std::string msg = std::to_string(pin);
                        GlobalLogger.Error(F("CheckIfPinAvailable error - pin allready reserved: "),msg.c_str());
                        return false;
                    }
                    if ((pinMode & pinDef.mode) == pinMode)
                        return true;
                    std::string errStr = Convert::toBin(pinMode) + " & " + Convert::toBin(pinDef.mode);
                    GlobalLogger.Error(F("CheckIfPinAvailable error - pinmode mismatch: "),errStr.c_str());
                    return false;
                }
            }
            std::string msg = std::to_string(pin);
            GlobalLogger.Error(F("Pin to reserve - not found: "),msg.c_str());
            return false;
            //return HAL_JSON_VERIFY_JSON_RETURN_OK;
        }

        bool CheckIfPinAvailableAndReserve(uint8_t pin, uint8_t pinMode) {
            if (CheckIfPinAvailable(pin, pinMode)) {
                ReservePin(pin);
                return true;
            }
            return false;
        }

        bool ValidateJsonAndCheckIfPinAvailableAndReserve(const JsonVariant& jsonObj, uint8_t pinMode) {
            if (jsonObj.containsKey(HAL_JSON_KEYNAME_PIN) == false) { GlobalLogger.Error(HAL_JSON_ERR_MISSING_KEY(HAL_JSON_KEYNAME_PIN)); return false; }
            if (jsonObj[HAL_JSON_KEYNAME_PIN].is<uint8_t>() == false)  { GlobalLogger.Error(HAL_JSON_ERR_VALUE_TYPE(HAL_JSON_KEYNAME_PIN)); return false; }
            uint8_t pin = jsonObj[HAL_JSON_KEYNAME_PIN].as<uint8_t>(); 
            return GPIO_manager::CheckIfPinAvailableAndReserve(pin, static_cast<uint8_t>(GPIO_manager::PinMode::OUT));
        }

        void ClearAllReservations() {
            if (available_gpio_list_lenght == -1) set_available_gpio_list_length();
            if (reservedPins == nullptr)
                reservedPins = new uint8_t[available_gpio_list_lenght];
            for (int i=0;i<available_gpio_list_lenght;i++)
                reservedPins[i] = 0;
        }
        /** it's recommended to call CheckIfPinAvailable prior to using this function,
         * this function is very basic and do only set the actual pin to reserved state, 
         * so calling it many times on the same pin have no effect */
        void ReservePin(uint8_t pin) {
            for (int i=0;i<available_gpio_list_lenght;i++) {
                if (available_gpio_list[i].pin == pin) {
                    reservedPins[i] = 1;
                    return;
                }
            }
        }
    }
}