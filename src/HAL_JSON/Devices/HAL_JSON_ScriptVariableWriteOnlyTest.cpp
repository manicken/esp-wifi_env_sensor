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

#include "HAL_JSON_ScriptVariableWriteOnlyTest.h"

namespace HAL_JSON {
    
    ScriptVariableWriteOnlyTest::ScriptVariableWriteOnlyTest(const JsonVariant &jsonObj, const char* type) : Device(UIDPathMaxLength::One,type) {
        uid = encodeUID(GetAsConstChar(jsonObj,HAL_JSON_KEYNAME_UID));
        value = GetAsUINT32(jsonObj, "val",0);
    }

    bool ScriptVariableWriteOnlyTest::VerifyJSON(const JsonVariant &jsonObj) {
        // could add type def later if wanted
        // also nonvolatile storage could be a mode
        return true;
    }

    Device* ScriptVariableWriteOnlyTest::Create(const JsonVariant &jsonObj, const char* type) {
        return new ScriptVariableWriteOnlyTest(jsonObj, type);
    }

    HALOperationResult ScriptVariableWriteOnlyTest::write(const HALValue& val) {
        value = val;
        return HALOperationResult::Success;
    }

    String ScriptVariableWriteOnlyTest::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += type;
        ret += "\",";
        ret += DeviceConstStrings::value;//StartWithComma;
        if (value.getType() == HALValue::Type::FLOAT)
            ret += std::to_string(value.asFloat()).c_str();
        else if (value.getType() == HALValue::Type::UINT)
            ret += std::to_string(value.asUInt()).c_str();
        else if (value.getType() == HALValue::Type::INT)
            ret += std::to_string(value.asInt()).c_str();
        else
            ret += "\"not set\"";
        return ret;
    }

}