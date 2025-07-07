
#include "HAL_JSON_TX433unit.h"

namespace HAL_JSON {

    bool TX433unit::VerifyFC_JSON(const JsonVariant &jsonObj) {
        if (!ValidateJsonStringField(jsonObj,"ch")) return false;
        if (!ValidateJsonStringField(jsonObj,"btn")) return false;
        //if (!ValidateJsonStringField(jsonObj,"state")) return false;
        return true;
    }
    
    TX433unit::TX433unit(const JsonVariant &jsonObj, const char* type) : Device(UIDPathMaxLength::One,type) {
        const char* uidStr = jsonObj[HAL_JSON_KEYNAME_UID].as<const char*>();
        uid = encodeUID(uidStr);
        const char* model = GetAsConstChar(jsonObj, HAL_JSON_KEYNAME_TX433_MODEL);
        
        if (CharArray::equalsIgnoreCase(model, "lc"))
            staticData = RF433::Get433_LC_Data(jsonObj);
        else if (CharArray::equalsIgnoreCase(model, "sfc"))
            staticData = RF433::Get433_SFC_Data(jsonObj);
        else if (CharArray::equalsIgnoreCase(model, "afc"))
            staticData = RF433::Get433_AFC_Data(jsonObj);
        //else this will never happen if VerifyJSON is used beforehand

        fixedState = (jsonObj.containsKey(HAL_JSON_KEYNAME_TX433_STATE) && IsUINT32(jsonObj,HAL_JSON_KEYNAME_TX433_STATE));
    }

    bool TX433unit::VerifyJSON(const JsonVariant &jsonObj) {
        if (ValidateJsonStringField(jsonObj, HAL_JSON_KEYNAME_UID) == false) { return false; }
        if (ValidateJsonStringField(jsonObj, HAL_JSON_KEYNAME_TX433_MODEL) == false) { return false; }

        const char* model = GetAsConstChar(jsonObj, HAL_JSON_KEYNAME_TX433_MODEL);

        if (CharArray::equalsIgnoreCase(model, "lc")) {
            if (!RF433::VerifyLC_JSON(jsonObj)) return false;
        } else if (CharArray::equalsIgnoreCase(model, "sfc")) {
           if (!VerifyFC_JSON(jsonObj)) return false;
        } else if (CharArray::equalsIgnoreCase(model, "afc")) {
           if (!VerifyFC_JSON(jsonObj)) return false;
        } else {
            GlobalLogger.Error(F("TX433unit - invalid model type: "),model);
            return false;
        }
        return true;
    }

    bool TX433unit::write(const HALValue &val) {
        if (model == TX433_MODEL::FixedCode) {
            if (fixedState == false)
                RF433::SendTo433_FC(staticData, val.asUInt());
            else
                RF433::SendTo433_FC(staticData);
            return true;
        }
        else if (model == TX433_MODEL::LearningCode) {
            if (fixedState == false)
                RF433::SendTo433_LC(staticData, val.asUInt());
            else
                RF433::SendTo433_LC(staticData);
            return true;
        }
        return false; // this will never happend
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
        ret += String(staticData, 16);
        ret += "\"";
        return ret;
    }

}