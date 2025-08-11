
#include "HAL_JSON_DHT.h"

namespace HAL_JSON {

    bool DHT::isValidDHTModel(const char* modelStr) {
        return
            CharArray::equalsIgnoreCase(modelStr, HAL_JSON_TYPE_DHT_MODEL_DHT11)  ||
            CharArray::equalsIgnoreCase(modelStr, HAL_JSON_TYPE_DHT_MODEL_DHT22)  ||
            CharArray::equalsIgnoreCase(modelStr, HAL_JSON_TYPE_DHT_MODEL_AM2302) ||
            CharArray::equalsIgnoreCase(modelStr, HAL_JSON_TYPE_DHT_MODEL_RHT03);
    }

    DHT::DHT(const JsonVariant &jsonObj, const char* type) : Device(UIDPathMaxLength::One, type) {
        //const char* uidStr = jsonObj[HAL_JSON_KEYNAME_UID].as<const char*>();
        //uid = encodeUID(uidStr);
        uid = encodeUID(GetAsConstChar(jsonObj,HAL_JSON_KEYNAME_UID));
        //pin = jsonObj[HAL_JSON_KEYNAME_PIN].as<uint8_t>();
        pin = GetAsUINT32(jsonObj, HAL_JSON_KEYNAME_PIN);
        GPIO_manager::ReservePin(pin); // this is in most cases taken care of in VerifyJSON but there are situations where it's needed
        refreshTimeMs = ParseRefreshTimeMs(jsonObj, HAL_JSON_TYPE_DHT_DEFAULT_REFRESHRATE_MS);
        if (refreshTimeMs < HAL_JSON_TYPE_DHT_DEFAULT_REFRESHRATE_MS) refreshTimeMs = HAL_JSON_TYPE_DHT_DEFAULT_REFRESHRATE_MS;
        //const char* modelStr = jsonObj[HAL_JSON_KEYNAME_DHT_MODEL].as<const char*>();
        const char* modelStr = GetAsConstChar(jsonObj, HAL_JSON_KEYNAME_DHT_MODEL);
        DHTesp::DHT_MODEL_t model = DHTesp::DHT_MODEL_t::AUTO_DETECT; // auto detect will not be used but we can use it as a default here;
        if (CharArray::equalsIgnoreCase(modelStr, HAL_JSON_TYPE_DHT_MODEL_DHT11))
            model = DHTesp::DHT_MODEL_t::DHT11;
        else if (CharArray::equalsIgnoreCase(modelStr, HAL_JSON_TYPE_DHT_MODEL_DHT22))
            model = DHTesp::DHT_MODEL_t::DHT22;
        else if (CharArray::equalsIgnoreCase(modelStr, HAL_JSON_TYPE_DHT_MODEL_AM2302))
            model = DHTesp::DHT_MODEL_t::AM2302;
        else if (CharArray::equalsIgnoreCase(modelStr, HAL_JSON_TYPE_DHT_MODEL_RHT03))
            model = DHTesp::DHT_MODEL_t::RHT03;
        dht.setup(pin,model);
        lastUpdateMs = millis();
        data = dht.getTempAndHumidity();
     }

    bool DHT::VerifyJSON(const JsonVariant &jsonObj) {
        if (ValidateJsonStringField(jsonObj, HAL_JSON_KEYNAME_DHT_MODEL) == false){ SET_ERR_LOC(HAL_JSON_ERROR_SOURCE_DHT_VERIFY_JSON); return false; }
        //const char* modelStr = jsonObj[HAL_JSON_KEYNAME_DHT_MODEL].as<const char*>();
        const char* modelStr = GetAsConstChar(jsonObj, HAL_JSON_KEYNAME_DHT_MODEL); // this take 8bytes less than above

        if (!modelStr || isValidDHTModel(modelStr) == false) {
            GlobalLogger.Error(F("DHT model invalid/missing: "), modelStr);
            return false;
        }
        return GPIO_manager::ValidateJsonAndCheckIfPinAvailableAndReserve(jsonObj, (static_cast<uint8_t>(GPIO_manager::PinMode::OUT) | static_cast<uint8_t>(GPIO_manager::PinMode::IN)));
    }

    Device* DHT::Create(const JsonVariant &jsonObj, const char* type) {
        return new DHT(jsonObj, type);
    }

    String DHT::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += type;
        ret += "\"";
        ret += DeviceConstStrings::pin;
        ret += std::to_string(pin).c_str();
        ret += ",\"humidity\":";
        ret += std::to_string(data.humidity).c_str();
        ret += ",\"temperature\":";
        ret += std::to_string(data.temperature).c_str();
        return ret;
    }

    void DHT::loop() {
        uint32_t now = millis();
        if (now - lastUpdateMs >= refreshTimeMs) {
            lastUpdateMs = millis();
            data = dht.getTempAndHumidity(); // this could take up to 250mS (of what i have read, but the timing spec only make it to max ~23mS)
            //humidityValue = data.humidity;

        }
    }

    HALOperationResult  DHT::read(HALValue &val) {
        val = data.humidity;
        return HALOperationResult::Success;
    }

    Device::ReadToHALValue_FuncType DHT::GetReadToHALValue_Function(const char* funcName) {
        if (strcmp(funcName, "temp") == 0) {
            return DHT::readTemperature;
        } else if (strcmp(funcName, "humidity") == 0) {
            return DHT::readHumidity;
        }
        else {
            GlobalLogger.Warn(F("DHT::read - cmd not found: "), funcName); // this can then be read by getting the last entry from logger
            return nullptr;
        }
    }

    HALOperationResult DHT::readTemperature(Device* context, HALValue& val) {
        DHT* dht = static_cast<DHT*>(context);
        val = dht->data.temperature;
        return HALOperationResult::Success;
    }
    HALOperationResult DHT::readHumidity(Device* context, HALValue& val) {
        DHT* dht = static_cast<DHT*>(context);
        val = dht->data.humidity;
        return HALOperationResult::Success;
    }

    HALOperationResult  DHT::read(const HALReadValueByCmd &val) {
        if (val.cmd == "temp") {
            val.out_value = data.temperature;
            return HALOperationResult::Success;
        } else if (val.cmd == "humidity") {
            val.out_value = data.humidity;
            return HALOperationResult::Success;
        }
        else {
            std::string stdStrCmd = val.cmd.ToString();
            GlobalLogger.Warn(F("DHT::read - cmd not found: "), stdStrCmd.c_str()); // this can then be read by getting the last entry from logger
            return HALOperationResult::UnsupportedCommand;
        }
    }

    HALOperationResult  DHT::read(const HALReadStringRequestValue &val) {
        if (val.cmd == "temp") {
            val.out_value = "{\"temp\":" + std::to_string(data.temperature) + "}";
            return HALOperationResult::Success;
        } else if (val.cmd == "humidity") {
            val.out_value = "{\"humidity\":" + std::to_string(data.humidity) + "}";
            return HALOperationResult::Success;
        }
        else {
            std::string stdStrCmd = val.cmd.ToString();
            GlobalLogger.Warn(F("DHT::read - cmd not found: "), stdStrCmd.c_str()); // this can then be read by getting the last entry from logger
            return HALOperationResult::UnsupportedCommand;
        }
    }
    
}