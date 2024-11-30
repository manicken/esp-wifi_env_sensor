#include <Arduino.h>
#include <ArduinoJson.h>
#include <stdlib.h>
#include <LittleFS.h>
#include "LittleFS_ext.h"

/*
    this file is only intended to manage which functions that are assigned to a specific GPIO pin
    that table is stored into a JSON file
    this will also contain the webserver functions, such as retrival of available GPIO pins

    in the client UI:
    GPIO pin selection should be from a predefined list of available GPIO:s
    so that GPIO:s that are assigned to fixed hardware functions such as
    I2C,UART,SPI,SD-card, etc.
    do not collide 
*/
namespace GPIO_manager
{
    #define GPIO_MANAGER_FILES_PATH                 F("/GPIO_manager")
    #define GPIO_MANAGER_CONFIG_JSON_FILE           F("/GPIO_manager/cfg.json")
#if defined(ESP8266)
    uint8_t available_gpio_list[] {
        //0,  // (reserved for programming)
        //1,  // TXD0 (reserved for programming/UART)
        //2,  // TXD1 (reserved for debug)
        //3,  // RXD0 (reserved for programming/UART)
        4,    // I2C SDA
        5,    // I2C SCL
        //6,  // SDCLK (reserved for spi flash)
        //7,  // SDD0 (reserved for spi flash)
        //8,  // SDD1 (reserved for spi flash)
        //9,  // SDD2 (reserved for spi flash)
        //10, // SDD3 (reserved for spi flash)
        //11, // SDCMD (reserved for spi flash)
        12, // SPI MISO
        13, // SPI MOSI
        14, // SPI SCLK
        15, // SPI CS/TXD2
    };
#elif defined(ESP32)
    uint8_t available_gpio_list[] {
        //0, // ADC2_1/TOUCH1 (reserved for programming)
        //1, // U0_TXD (reserved for programmer/debug)
        //2, // ADC2_2/TOUCH2/SD_DATA0 (must be LOW during boot/is connected to onboard LED, could be a output function only pin)
        //3, // U0_RXD (reserved for programmer/debug)
        4,  // ADC2_0/TOUCH0/SD_DATA1
        //5,  // VSPI_CS (must be HIGH during boot)
        //6,  // U1_CTS/SPI_CLK (reserved for flash)
        //7,  // U2_RTS/SPI_MISO (reserved for flash)
        //8,  // U2_CTS/SPI_MOSI (reserved for flash)
        //9,  // U1_RXD/SPI_HD (reserved for flash)
        //10, // U1_TXD/SPI_WP (reserved for flash)
        //11, // U1_RTX/SPI_CS (reserved for flash)
        //12, // ADC2_5/TOUCH5/HSPI_MISO/SD_DATA2 (must be LOW during boot, could be a output function only pin)
        13, // ADC2_4/TOUCH4/HSPI_MOSI/SD_DATA3
        14, // ADC2_6/TOUCH6/HSPI_CLK/SD_CLK
        //15, // ADC2_3/TOUCH3/HSPI_CS/SD_CMD (must be HIGH during boot, could be a output function only pin)
        16, // U2_RXD
        17, // U2_TXD
        18, // VSPI_CLK
        19, // VSPI_MISO
        21, // I2C_SDA
        22, // I2C_SCL
        23, // VSPI_MOSI
        25, // ADC2_8/DAC1
        26, // ADC2_9/DAC2
        27, // ADC2_7/TOUCH7
        32, // ADC1_4/TOUCH9/XTAL32
        33, // ADC1_5/TOUCH8/XTAL32
        //34, // ADC1_6 (input only)
        //35, // ADC1_7 (input only)
        //36, // ADC1_0/SensVP (input only)
        //39  // ADC1_3/SensVN (input only)
        };
#endif
}