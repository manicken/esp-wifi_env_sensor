#include "GPIO_manager.h"

namespace GPIO_manager
{
    #ifdef ESP8266
    ESP8266WebServer *server = nullptr;
#elif defined(ESP32)
    fs_WebServer *server = nullptr;
#endif
    
#if defined(ESP8266)
    const gpio_pin available_gpio_list[] {
        {0,PinMode::OUT},  // (reserved for programming) only safe to use as a output
        {1,PinMode::OUT},  // TXD0 (reserved for programming/UART) only safe to use as a output
        {2,PinMode::OUT},  // TXD1 (reserved for debug) only safe to use as a output
        //{3,PinMode::NA},  // RXD0 (reserved for programming/UART)
        {4,PinMode::IO},    // I2C SDA
        {5,PinMode::IO},    // I2C SCL
        //{6,PinMode::NA},  // SDCLK (reserved for spi flash)
        //{7,PinMode::NA},  // SDD0 (reserved for spi flash)
        //{8,PinMode::NA},  // SDD1 (reserved for spi flash)
        //{9,PinMode::NA},  // SDD2 (reserved for spi flash)
        //{10,PinMode::NA}, // SDD3 (reserved for spi flash)
        //{11,PinMode::NA}, // SDCMD (reserved for spi flash)
        {12,PinMode::IO}, // SPI MISO
        {13,PinMode::IO}, // SPI MOSI
        {14,PinMode::IO}, // SPI SCLK
        {15,PinMode::IO}, // SPI CS/TXD2
    };
#elif defined(ESP32)
    const gpio_pin available_gpio_list[] {
        //{0,PinMode::OUT}, // ADC2_1/TOUCH1 (reserved for programming)
        //{1,PinMode::OUT}, // U0_TXD (reserved for programmer/debug)
        {2,PinMode::OUT}, // ADC2_2/TOUCH2/SD_DATA0 (must be LOW during boot/is connected to onboard LED, could be a output function only pin)
        //{3,PinMode::NA}, // U0_RXD (reserved for programmer/debug cannot be shared directly)
        {4,PinMode::IO},  // ADC2_0/TOUCH0/SD_DATA1
        {5,PinMode::OUT},  // VSPI_CS (must be HIGH during boot)
        //{6,PinMode::NA},  // U1_CTS/SPI_CLK (reserved for flash)
        //{7,PinMode::NA},  // U2_RTS/SPI_MISO (reserved for flash)
        //{8,PinMode::NA},  // U2_CTS/SPI_MOSI (reserved for flash)
        //{9,PinMode::NA},  // U1_RXD/SPI_HD (reserved for flash)
        //{10,PinMode::NA}, // U1_TXD/SPI_WP (reserved for flash)
        //{11,PinMode::NA}, // U1_RTX/SPI_CS (reserved for flash)
        {12,PinMode::OUT}, // ADC2_5/TOUCH5/HSPI_MISO/SD_DATA2 (must be LOW during boot, could be a output function only pin)
        {13,PinMode::IO}, // ADC2_4/TOUCH4/HSPI_MOSI/SD_DATA3
        {14,PinMode::IO}, // ADC2_6/TOUCH6/HSPI_CLK/SD_CLK
        {15,PinMode::OUT}, // ADC2_3/TOUCH3/HSPI_CS/SD_CMD (must be HIGH during boot, could be a output function only pin)
        {16,PinMode::IO}, // U2_RXD
        {17,PinMode::IO}, // U2_TXD
        {18,PinMode::IO}, // VSPI_CLK
        {19,PinMode::IO}, // VSPI_MISO
        {21,PinMode::IO}, // I2C_SDA
        {22,PinMode::IO}, // I2C_SCL
        {23,PinMode::IO}, // VSPI_MOSI
        {25,PinMode::IO}, // ADC2_8/DAC1
        {26,PinMode::IO}, // ADC2_9/DAC2
        {27,PinMode::IO}, // ADC2_7/TOUCH7
        {32,PinMode::IO}, // ADC1_4/TOUCH9/XTAL32
        {33,PinMode::IO}, // ADC1_5/TOUCH8/XTAL32
        {34,PinMode::IN}, // ADC1_6 (input only)
        {35,PinMode::IN}, // ADC1_7 (input only)
        {36,PinMode::IN}, // ADC1_0/SensVP (input only)
        {39,PinMode::IN}  // ADC1_3/SensVN (input only)
        };
#endif
    const uint8_t available_gpio_list_lenght = sizeof(available_gpio_list)/sizeof(available_gpio_list[0]);
    const uint8_t PinModeStrings_lenght = sizeof(PinModeStrings)/sizeof(PinModeStrings[0]);


    #if defined(ESP8266)
    void setup(ESP8266WebServer &srv) {
#elif defined(ESP32)
    void setup(fs_WebServer &srv) {
#endif
        server = &srv;
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
        for (int i=0;i<PinModeStrings_lenght;i++)
        {
            /*
            srv_return_msg.concat("{\"mode\":");
            srv_return_msg.concat((int8_t)PinModeStrings[i].mode);
            srv_return_msg.concat(",\"str\":\"");
            srv_return_msg.concat(PinModeStrings[i].Name);
            srv_return_msg.concat("\"}");
            */
            srv_return_msg.concat("\"");
            srv_return_msg.concat((int8_t)PinModeStrings[i].mode);
            srv_return_msg.concat("\":\"");
            srv_return_msg.concat(PinModeStrings[i].Name);
            srv_return_msg.concat("\"");
            if (i<(PinModeStrings_lenght-1))
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
        server->send(200, "text/json", srv_return_msg);
    }
}