/*

  <one line to give the program's name and a brief idea of what it does.>

  Copyright (C) 2025 Jannik Svensson

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or 
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License 
  along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once


#include <Arduino.h> // Needed for String class

#include <ArduinoJson.h>
#include <stdlib.h>

#include "../Support/Logger.h"
#include "../Support/ConvertHelper.h"
#include "HAL_JSON_ZeroCopyString.h"

#include "HAL_JSON_Device_GlobalDefines.h"

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

#define HAL_JSON_CMD_EXEC_GPIO_LIST_MODE_STRING   "string"
#define HAL_JSON_CMD_EXEC_GPIO_LIST_MODE_HEX      "hex"
#define HAL_JSON_CMD_EXEC_GPIO_LIST_MODE_BINARY   "binary"

namespace HAL_JSON {
    namespace GPIO_manager
    {
        

        enum class PinMode : uint8_t {
            Reserved = 0x01,
            SpecialAtBoot = 0x02,
            LOW2BOOT = 0x04,
            HIGH2BOOT = 0x08,        
            OUT = 0x10,
            IN = 0x20,
            AIN = 0x40,
            AOUT = 0x80
        };

        typedef struct {
            const char* Name;
            uint8_t mode;
        } PinModeDef;

        enum class PrintListMode {
            String,
            Hex,
            Binary
        };

        typedef struct {
            uint8_t pin;
            uint8_t mode;
        } gpio_pin;

        extern const gpio_pin available_gpio_list[];
        extern int available_gpio_list_lenght;
        void set_available_gpio_list_length();

        extern const PinModeDef PinModeStrings[];
        extern int PinModeStrings_length;
        void set_PinModeStrings_length();

        std::string describePinMode(uint8_t mask);
        
        bool CheckIfPinAvailableAndReserve(uint8_t pin, uint8_t pinMode);
        /** this is a nice function that can be used */
        bool ValidateJsonAndCheckIfPinAvailableAndReserve(const JsonVariant& jsonObj, uint8_t pinMode);
        bool ValidateJsonAndCheckIfPinAvailableAndReserve(const JsonVariant& jsonObj, const char* NAME, uint8_t pinMode);
        bool CheckIfPinAvailable(uint8_t pin, uint8_t pinMode);
        void ClearAllReservations();
        /** it's recommended to call CheckIfPinAvailable prior to using this function,
         * this function is very basic and do only set the actual pin to reserved state, 
         * so calling it many times on the same pin do not matter */
        void ReservePin(uint8_t pin);

        std::string GetList(ZeroCopyString& zcMode);
    }
}