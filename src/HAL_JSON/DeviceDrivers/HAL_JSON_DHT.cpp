
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
        pin = jsonObj[HAL_JSON_KEYNAME_PIN].as<uint8_t>();
        GPIO_manager::ReservePin(pin); // this is in most cases taken care of in VerifyJSON but there are situations where it's needed
        refreshTimeMs = ParseRefreshTimeMs(jsonObj, HAL_JSON_TYPE_DHT_DEFAULT_REFRESHRATE_MS);
        if (refreshTimeMs < HAL_JSON_TYPE_DHT_DEFAULT_REFRESHRATE_MS) refreshTimeMs = HAL_JSON_TYPE_DHT_DEFAULT_REFRESHRATE_MS;
        const char* modelStr = jsonObj[HAL_JSON_KEYNAME_DHT_MODEL].as<const char*>();
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
        if (ValidateJsonStringField(jsonObj, HAL_JSON_KEYNAME_DHT_MODEL) == false) return false;
        const char* modelStr = jsonObj[HAL_JSON_KEYNAME_DHT_MODEL].as<const char*>();
        if (isValidDHTModel(modelStr) == false) {
            GlobalLogger.Error(F("DHT model invalid: "), modelStr);
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
        ret += DeviceConstStrings::type;
        ret += type;
        ret += "\"";
        ret += DeviceConstStrings::pin;
        ret += pin;
        ret += ",\"humidity\":";
        ret += data.humidity;
        ret += ",\"temperature\":";
        ret += data.temperature;
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

    bool DHT::read(HALValue &val) {
        val = data.humidity;
        return true;
    }

    bool DHT::read(const HALReadValueByCmd &val) {
        if (val.cmd == "temp") {
            val.out_value = data.temperature;
            return true;
        } else if (val.cmd == "humidity") {
            val.out_value = data.humidity;
            return true;
        }
        else {
            GlobalLogger.Warn(F("DHT::read - cmd not found: "), val.cmd.c_str()); // this can then be read by getting the last entry from logger
            return false;
        }
    }

    bool DHT::read(const HALReadStringRequestValue &val) {
        if (val.cmd == "temp") {
            val.out_value = "{\"value\":" + String(data.temperature) + "}";
            return true;
        } else if (val.cmd == "humidity") {
            val.out_value = "{\"value\":" + String(data.humidity) + "}";
            return true;
        }
        else {
            GlobalLogger.Warn(F("DHT::read - cmd not found: "), val.cmd.c_str()); // this can then be read by getting the last entry from logger
            return false;
        }
    }
    
}