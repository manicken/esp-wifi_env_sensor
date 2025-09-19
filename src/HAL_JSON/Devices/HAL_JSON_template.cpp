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

#include "HAL_JSON_template.h"

namespace HAL_JSON {
    
    Template::Template(const JsonVariant &jsonObj, const char* type) : Device(UIDPathMaxLength::One,type) {

    }

    bool Template::VerifyJSON(const JsonVariant &jsonObj) {
        // this is a check only to verify that the pin cfg exist
        return GPIO_manager::ValidateJsonAndCheckIfPinAvailableAndReserve(jsonObj, (static_cast<uint8_t>(GPIO_manager::PinMode::OUT) | static_cast<uint8_t>(GPIO_manager::PinMode::IN)));
    }

    Device* Template::Create(const JsonVariant &jsonObj, const char* type) {
        return new Template(jsonObj, type);
    }

    String Template::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += type;
        ret += "\",";
        ret += DeviceConstStrings::pin;
        ret += std::to_string(pin).c_str();
        return ret;
    }

    void Template::loop() {}
    void Template::begin() {}
    Device* Template::findDevice(UIDPath& path) { return nullptr; }

    HALOperationResult Template::read(HALValue& val) { return HALOperationResult::UnsupportedOperation; }
    HALOperationResult Template::write(const HALValue& val) { return HALOperationResult::UnsupportedOperation; };
    HALOperationResult Template::read(const HALReadStringRequestValue& val) { return HALOperationResult::UnsupportedOperation; }
    HALOperationResult Template::write(const HALWriteStringRequestValue& val) { return HALOperationResult::UnsupportedOperation; }
    HALOperationResult Template::read(const HALReadValueByCmd& val) { return HALOperationResult::UnsupportedOperation; }
    HALOperationResult Template::write(const HALWriteValueByCmd& val) { return HALOperationResult::UnsupportedOperation; }
    HALOperationResult Template::exec() { return HALOperationResult::UnsupportedOperation; }
    HALOperationResult Template::exec(ZeroCopyString& cmd) { return HALOperationResult::UnsupportedOperation; }
    Device::ReadToHALValue_FuncType Template::GetReadToHALValue_Function(ZeroCopyString& zcFuncName) { return nullptr; }
    Device::ReadToHALValue_FuncType Template::GetWriteFromHALValue_Function(ZeroCopyString& zcFuncName) { return nullptr; }
    
    HALValue* Template::GetValueDirectAccessPtr() { return nullptr; }
}