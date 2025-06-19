#include "GPIO_manager.h"

#define MAKE_PIN_MASK_3(a, b, c) (static_cast<uint8_t>(a) | static_cast<uint8_t>(b) | static_cast<uint8_t>(c))
#define MAKE_PIN_MASK_2(a, b)    (static_cast<uint8_t>(a) | static_cast<uint8_t>(b))
#define MAKE_PIN_MASK_1(a)       (static_cast<uint8_t>(a))

namespace GPIO_manager
{
    WEBSERVER_TYPE *webserver = nullptr;

    
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
        //{1, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::SpecialAtBoot)}, // U0_TXD (reserved for programmer/debug)
        {2, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::LOW2BOOT)}, // ADC2_2/TOUCH2/SD_DATA0 (must be LOW during boot/is connected to onboard LED, could be a output function only pin)
        //{3, MAKE_PIN_MASK_2(PinMode::Reserved, PinMode::SpecialAtBoot)}, // U0_RXD (reserved for programmer/debug cannot be shared directly)
        {4, MAKE_PIN_MASK_3(PinMode::OUT, PinMode::IN, PinMode::AIN)},  // ADC2_0/TOUCH0/SD_DATA1
        {5, MAKE_PIN_MASK_3(PinMode::OUT, PinMode::HIGH2BOOT, PinMode::SpecialAtBoot)},  // VSPI_CS (must be HIGH during boot better to keep it a output only)
        //{6, MAKE_PIN_MASK_2(PinMode::Reserved, PinMode::SpecialAtBoot)},  // U1_CTS/SPI_CLK (reserved for flash)
        //{7, MAKE_PIN_MASK_2(PinMode::Reserved, PinMode::SpecialAtBoot)},  // U2_RTS/SPI_MISO (reserved for flash)
        //{8, MAKE_PIN_MASK_2(PinMode::Reserved, PinMode::SpecialAtBoot)},  // U2_CTS/SPI_MOSI (reserved for flash)
        //{9, MAKE_PIN_MASK_2(PinMode::Reserved, PinMode::SpecialAtBoot)},  // U1_RXD/SPI_HD (reserved for flash)
        //{10, MAKE_PIN_MASK_2(PinMode::Reserved, PinMode::SpecialAtBoot)}, // U1_TXD/SPI_WP (reserved for flash)
        //{11, MAKE_PIN_MASK_2(PinMode::Reserved, PinMode::SpecialAtBoot)}, // U1_RTX/SPI_CS (reserved for flash)
        {12, MAKE_PIN_MASK_3(PinMode::OUT, PinMode::LOW2BOOT, PinMode::SpecialAtBoot)}, // ADC2_5/TOUCH5/HSPI_MISO/SD_DATA2 (must be LOW during boot, could be a output function only pin)
        {13, MAKE_PIN_MASK_3(PinMode::OUT, PinMode::IN, PinMode::AIN)}, // ADC2_4/TOUCH4/HSPI_MOSI/SD_DATA3
        {14, MAKE_PIN_MASK_3(PinMode::OUT, PinMode::IN, PinMode::AIN)}, // ADC2_6/TOUCH6/HSPI_CLK/SD_CLK
        {15, MAKE_PIN_MASK_3(PinMode::OUT, PinMode::HIGH2BOOT, PinMode::SpecialAtBoot)}, // ADC2_3/TOUCH3/HSPI_CS/SD_CMD (must be HIGH during boot, could be a output function only pin)
        {16, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::IN)}, // U2_RXD
        {17, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::IN)}, // U2_TXD
        {18, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::IN)}, // VSPI_CLK
        {19, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::IN)}, // VSPI_MISO
        {21, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::IN)}, // I2C_SDA
        {22, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::IN)}, // I2C_SCL
        {23, MAKE_PIN_MASK_2(PinMode::OUT, PinMode::IN)}, // VSPI_MOSI
        {25, MAKE_PIN_MASK_3(PinMode::OUT, PinMode::IN, PinMode::AIN)}, // ADC2_8/DAC1
        {26, MAKE_PIN_MASK_3(PinMode::OUT, PinMode::IN, PinMode::AIN)}, // ADC2_9/DAC2
        {27, MAKE_PIN_MASK_3(PinMode::OUT, PinMode::IN, PinMode::AIN)}, // ADC2_7/TOUCH7
        {32, MAKE_PIN_MASK_3(PinMode::OUT, PinMode::IN, PinMode::AIN)}, // ADC1_4/TOUCH9/XTAL32
        {33, MAKE_PIN_MASK_3(PinMode::OUT, PinMode::IN, PinMode::AIN)}, // ADC1_5/TOUCH8/XTAL32
        {34, MAKE_PIN_MASK_2(PinMode::IN, PinMode::AIN)}, // ADC1_6 (input only)
        {35, MAKE_PIN_MASK_2(PinMode::IN, PinMode::AIN)}, // ADC1_7 (input only)
        {36, MAKE_PIN_MASK_2(PinMode::IN, PinMode::AIN)}, // ADC1_0/SensVP (input only)
        {39, MAKE_PIN_MASK_2(PinMode::IN, PinMode::AIN)}  // ADC1_3/SensVN (input only)
        };
#endif
    const uint8_t available_gpio_list_lenght = sizeof(available_gpio_list)/sizeof(available_gpio_list[0]);

    const PinModeDef PinModeStrings[] = {
        {"OUT", static_cast<uint8_t>(PinMode::OUT)},
        {"IN", static_cast<uint8_t>(PinMode::IN)},
        {"AIN", static_cast<uint8_t>(PinMode::AIN)},
        {"LOW2BOOT", static_cast<uint8_t>(PinMode::LOW2BOOT)},
        {"HIGH2BOOT", static_cast<uint8_t>(PinMode::HIGH2BOOT)},
        {"Reserved", static_cast<uint8_t>(PinMode::Reserved)},
        {"SpecialAtBoot", static_cast<uint8_t>(PinMode::SpecialAtBoot)}
    };
    const uint8_t PinModeStrings_length = sizeof(PinModeStrings)/sizeof(PinModeStrings[0]);

    uint8_t reservedPins[available_gpio_list_lenght];

    std::string describePinMode(uint8_t mask) {
        std::string result;
        for (size_t i = 0; i < PinModeStrings_length; ++i) {
            if (mask & PinModeStrings[i].mode) {
                if (!result.empty()) result += "|";
                result += PinModeStrings[i].Name;
            }
        }
        return result.empty() ? "None" : result;
    }

    void setup(WEBSERVER_TYPE &srv) {
        webserver = &srv;
        srv.on(GPIO_MANAGER_GET_AVAILABLE_GPIO_LIST, HTTP_GET, sendList);
    }

    void sendList()
    {
        String srv_return_msg = "{";
#if defined(ESP8266)
        srv_return_msg.concat("\"MCU\":\"ESP8266\",");
#elif defined(ESP32)
        srv_return_msg.concat("\"MCU\":\"ESP32\",");
#endif
        srv_return_msg.concat("\"PinModes\":{");
        for (int i=0;i<PinModeStrings_length;i++)
        {
            /*
            srv_return_msg.concat("{\"mode\":");
            srv_return_msg.concat((int8_t)PinModeStrings[i].mode);
            srv_return_msg.concat(",\"str\":\"");
            srv_return_msg.concat(PinModeStrings[i].Name);
            srv_return_msg.concat("\"}");
            */
            srv_return_msg.concat("\"");
            
            srv_return_msg.concat(describePinMode((uint8_t)PinModeStrings[i].mode).c_str());
            srv_return_msg.concat("\":\"");
            srv_return_msg.concat(PinModeStrings[i].Name);
            srv_return_msg.concat("\"");
            if (i<(PinModeStrings_length-1))
                srv_return_msg.concat(",");
        }
        srv_return_msg.concat("},");
        srv_return_msg.concat("\"list\":{");
        for (int i=0;i<available_gpio_list_lenght;i++)
        {
            /*srv_return_msg.concat("{\"pin\":");
            srv_return_msg.concat(available_gpio_list[i].pin);
            srv_return_msg.concat(",\"mode\":");
            srv_return_msg.concat((int8_t)available_gpio_list[i].mode);
            srv_return_msg.concat("}");*/
            srv_return_msg.concat("\"");
            srv_return_msg.concat(available_gpio_list[i].pin);
            srv_return_msg.concat("\":");
            srv_return_msg.concat((int8_t)available_gpio_list[i].mode);
            if (i<(available_gpio_list_lenght-1))
                srv_return_msg.concat(",");
        }
        srv_return_msg.concat("}");
        srv_return_msg.concat("}");
        webserver->send(200, "text/json", srv_return_msg);
    }

    HAL_JSON_VERIFY_JSON_RETURN_TYPE CheckIfPinAvailable(uint8_t pin, uint8_t pinMode) {
        for (int i=0;i<available_gpio_list_lenght;i++) {
            if (available_gpio_list[i].pin == pin) {
                if (reservedPins[i] == 1) return F("CheckIfPinAvailable error - pin allready reserved");

                if (pinMode & available_gpio_list[i].mode > 0)
                    return HAL_JSON_VERIFY_JSON_RETURN_OK;
                return F("CheckIfPinAvailable error - pinmode mismatch");
            }
        }
        return F("Pin to reserve - not found!");
        //return HAL_JSON_VERIFY_JSON_RETURN_OK;
    }

    void ClearAllReservations() {
        for (int i=0;i<available_gpio_list_lenght;i++)
            reservedPins[i] = 0;
    }
    /** CheckIfPinAvailable must be called prior to using this function. */
    void ReservePin(uint8_t pin) {
        for (int i=0;i<available_gpio_list_lenght;i++) {
            if (available_gpio_list[i].pin == pin) {
                reservedPins[i] = 1;
                return;
            }
        }
    }
}