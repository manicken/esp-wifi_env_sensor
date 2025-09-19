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

#include "HAL_JSON_TX433unit.h"

namespace HAL_JSON {

    bool TX433unit::VerifyFC_JSON(const JsonVariant &jsonObj) {
        if (!ValidateJsonStringField(jsonObj,"ch")) { SET_ERR_LOC(HAL_JSON_ERROR_SOURCE_TX433_UNIT_VERIFY_FC_JSON); return false; }
        if (!ValidateJsonStringField(jsonObj,"btn")) { SET_ERR_LOC(HAL_JSON_ERROR_SOURCE_TX433_UNIT_VERIFY_FC_JSON); return false; }
        //if (!ValidateJsonStringField(jsonObj,"state")) return false;
        return true;
    }
    
    TX433unit::TX433unit(const JsonVariant &jsonObj, const char* type, const uint32_t pin) : Device(UIDPathMaxLength::One,type), pin(pin) {
        const char* uidStr = jsonObj[HAL_JSON_KEYNAME_UID].as<const char*>();
        uid = encodeUID(uidStr);
        const char* modelStr = GetAsConstChar(jsonObj, HAL_JSON_KEYNAME_TX433_MODEL);
        
        if (CharArray::equalsIgnoreCase(modelStr, "lc"))
            staticData = RF433::Get433_LC_Data(jsonObj);
        else if (CharArray::equalsIgnoreCase(modelStr, "sfc"))
            staticData = RF433::Get433_SFC_Data(jsonObj);
        else if (CharArray::equalsIgnoreCase(modelStr, "afc"))
            staticData = RF433::Get433_AFC_Data(jsonObj);
        //else this will never happen if VerifyJSON is used beforehand

        fixedState = (jsonObj.containsKey(HAL_JSON_KEYNAME_TX433_STATE) && IsUINT32(jsonObj,HAL_JSON_KEYNAME_TX433_STATE));
    }

    bool TX433unit::VerifyJSON(const JsonVariant &jsonObj) {
        if (ValidateJsonStringField(jsonObj, HAL_JSON_KEYNAME_UID) == false) { SET_ERR_LOC(HAL_JSON_ERROR_SOURCE_TX433_UNIT_VERIFY_JSON); return false; }
        if (ValidateJsonStringField(jsonObj, HAL_JSON_KEYNAME_TX433_MODEL) == false) { SET_ERR_LOC(HAL_JSON_ERROR_SOURCE_TX433_UNIT_VERIFY_JSON); return false; }

        const char* modelStr = GetAsConstChar(jsonObj, HAL_JSON_KEYNAME_TX433_MODEL);

        if (CharArray::equalsIgnoreCase(modelStr, "lc")) {
            if (!RF433::VerifyLC_JSON(jsonObj)) return false;
        } else if (CharArray::equalsIgnoreCase(modelStr, "sfc")) {
           if (!VerifyFC_JSON(jsonObj)) return false;
        } else if (CharArray::equalsIgnoreCase(modelStr, "afc")) {
           if (!VerifyFC_JSON(jsonObj)) return false;
        } else {
            GlobalLogger.Error(F("TX433unit - invalid model type: "),modelStr);
            return false;
        }
        return true;
    }

    HALOperationResult TX433unit::write(const HALValue &val) {
        RF433::init(pin); // ensure that the correct pin is used and that it's set to a output
        if (model == TX433_MODEL::FixedCode) {
            if (fixedState == false)
                RF433::SendTo433_FC(staticData, val.asUInt());
            else
                RF433::SendTo433_FC(staticData);
            return HALOperationResult::Success;
        }
        else if (model == TX433_MODEL::LearningCode) {
            if (fixedState == false)
                RF433::SendTo433_LC(staticData, val.asUInt());
            else
                RF433::SendTo433_LC(staticData);
            return HALOperationResult::Success;
        }
        return HALOperationResult::ExecutionFailed; // this will never happend
    }

    String TX433unit::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        
        ret += "\"model\":\"";
        if (model == TX433_MODEL::LearningCode)
            ret += "LC";
        else if (model == TX433_MODEL::FixedCode)
            ret += "FC";
        
        ret += "\",";
        ret += "\"data\":\"";
        ret += Convert::toHex(staticData).c_str();
        ret += "\"";
        return ret;
    }

}