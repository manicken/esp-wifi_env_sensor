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

#include "../HAL_JSON_Device.h"
#include <Wire.h>

namespace HAL_JSON {

    typedef Device* (*I2C_HAL_DEVICE_CREATE_FUNC)(const JsonVariant &json, const char* type, TwoWire& wire);
    typedef bool (*I2C_HAL_DEVICE_VERIFY_JSON_FUNC)(const JsonVariant &json);

    typedef struct I2C_DeviceTypeDef {
        const char* typeName;
        I2C_HAL_DEVICE_CREATE_FUNC Create_Function;
        I2C_HAL_DEVICE_VERIFY_JSON_FUNC Verify_JSON_Function;
    } I2C_DeviceTypeDef ;

    extern const I2C_DeviceTypeDef I2C_DeviceRegistry[];
    const I2C_DeviceTypeDef* GetI2C_DeviceTypeDef(const char* type);

}