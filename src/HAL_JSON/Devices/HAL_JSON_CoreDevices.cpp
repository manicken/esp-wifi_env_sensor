
#include "HAL_JSON_CoreDevices.h"

namespace HAL_JSON {

    
    // ██████  ██  ██████  ██ ████████  █████  ██          ██ ███    ██ ██████  ██    ██ ████████ 
    // ██   ██ ██ ██       ██    ██    ██   ██ ██          ██ ████   ██ ██   ██ ██    ██    ██    
    // ██   ██ ██ ██   ███ ██    ██    ███████ ██          ██ ██ ██  ██ ██████  ██    ██    ██    
    // ██   ██ ██ ██    ██ ██    ██    ██   ██ ██          ██ ██  ██ ██ ██      ██    ██    ██    
    // ██████  ██  ██████  ██    ██    ██   ██ ███████     ██ ██   ████ ██       ██████     ██   

    Device* DigitalInput::Create(const JsonVariant &jsonObj, const char* type) {
        return new DigitalInput(jsonObj, type);
    }

    bool DigitalInput::VerifyJSON(const JsonVariant &jsonObj) {
        return GPIO_manager::ValidateJsonAndCheckIfPinAvailableAndReserve(jsonObj, static_cast<uint8_t>(GPIO_manager::PinMode::IN));
    }

    DigitalInput::DigitalInput(const JsonVariant &jsonObj, const char* type) : Device(UIDPathMaxLength::One, type) {
        pin = GetAsUINT32(jsonObj, HAL_JSON_KEYNAME_PIN);// jsonObj[HAL_JSON_KEYNAME_PIN];// | 0;//.as<uint8_t>();
        uid = encodeUID(GetAsConstChar(jsonObj, HAL_JSON_KEYNAME_UID));
        //pin = jsonObj[HAL_JSON_KEYNAME_PIN];//.as<uint8_t>();
        GPIO_manager::ReservePin(pin);
        
        //const char* uidStr = jsonObj[HAL_JSON_KEYNAME_UID];//.as<const char*>();
        //uid = encodeUID(uidStr);

        pinMode(pin, INPUT); // input
    }
#ifndef HAL_JSON_USE_EFFICIENT_FIND
    Device* DigitalInput::findDevice(UIDPath& path) {
        if (path.first() == uid) return this;
        else return nullptr;
    }
#endif
    bool DigitalInput::read(HALValue &val) {
        //val.set((uint32_t)digitalRead(pin));
        val = (uint32_t)digitalRead(pin);
        return true;
    }

    String DigitalInput::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += type;
        ret += DeviceConstStrings::pin;
        ret += std::to_string(pin).c_str();
        ret += ",";
        ret += DeviceConstStrings::value;//StartWithComma;
        ret += std::to_string(digitalRead(pin)).c_str();
        return ret;
    }

    // ██████  ██  ██████  ██ ████████  █████  ██           ██████  ██    ██ ████████ ██████  ██    ██ ████████ 
    // ██   ██ ██ ██       ██    ██    ██   ██ ██          ██    ██ ██    ██    ██    ██   ██ ██    ██    ██    
    // ██   ██ ██ ██   ███ ██    ██    ███████ ██          ██    ██ ██    ██    ██    ██████  ██    ██    ██    
    // ██   ██ ██ ██    ██ ██    ██    ██   ██ ██          ██    ██ ██    ██    ██    ██      ██    ██    ██    
    // ██████  ██  ██████  ██    ██    ██   ██ ███████      ██████   ██████     ██    ██       ██████     ██    
    
    Device* DigitalOutput::Create(const JsonVariant &jsonObj, const char* type) {
        return new DigitalOutput(jsonObj, type);
    }

    bool DigitalOutput::VerifyJSON(const JsonVariant &jsonObj) {
        return GPIO_manager::ValidateJsonAndCheckIfPinAvailableAndReserve(jsonObj, static_cast<uint8_t>(GPIO_manager::PinMode::OUT));
    }

    DigitalOutput::DigitalOutput(const JsonVariant &jsonObj, const char* type) : Device(UIDPathMaxLength::One, type) {
        pin = GetAsUINT32(jsonObj, HAL_JSON_KEYNAME_PIN);// jsonObj[HAL_JSON_KEYNAME_PIN];// | 0;//.as<uint8_t>();
        uid = encodeUID(GetAsConstChar(jsonObj, HAL_JSON_KEYNAME_UID));
        //pin = jsonObj[HAL_JSON_KEYNAME_PIN];//.as<uint8_t>();
        GPIO_manager::ReservePin(pin);

        //const char* uidStr = jsonObj[HAL_JSON_KEYNAME_UID];//.as<const char*>();
        //uid = encodeUID(uidStr);

        pinMode(pin, OUTPUT); // output
    }

    DigitalOutput::~DigitalOutput() { pinMode(pin, INPUT); /*input*/ } // release the pin
#ifndef HAL_JSON_USE_EFFICIENT_FIND
    Device* DigitalOutput::findDevice(UIDPath& path) {
        if (path.first() == uid) return this;
        else return nullptr;
    }
#endif
    bool DigitalOutput::read(HALValue &val) {
        //val.set(value); // read back the latest write value
        val = value;
        return true;
    }

    bool DigitalOutput::write(const HALValue &val) {
        value = val;//val.asUInt();
        digitalWrite(pin, value);
        return true;
    }

    String DigitalOutput::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += type;
        ret += "\"";
        ret += DeviceConstStrings::pin;
        ret += std::to_string(pin).c_str();
        ret += ",";
        ret += DeviceConstStrings::value;//StartWithComma;
        ret += std::to_string(value).c_str();
        return ret;
    }

    // ██████  ██    ██ ██      ███████ ███████      ██████  ██    ██ ████████ ██████  ██    ██ ████████ 
    // ██   ██ ██    ██ ██      ██      ██          ██    ██ ██    ██    ██    ██   ██ ██    ██    ██    
    // ██████  ██    ██ ██      ███████ █████       ██    ██ ██    ██    ██    ██████  ██    ██    ██    
    // ██      ██    ██ ██           ██ ██          ██    ██ ██    ██    ██    ██      ██    ██    ██    
    // ██       ██████  ███████ ███████ ███████      ██████   ██████     ██    ██       ██████     ██    

    Device* SinglePulseOutput::Create(const JsonVariant &jsonObj, const char* type) {
        return new SinglePulseOutput(jsonObj, type);
    }

    bool SinglePulseOutput::VerifyJSON(const JsonVariant &jsonObj) {
        return GPIO_manager::ValidateJsonAndCheckIfPinAvailableAndReserve(jsonObj, static_cast<uint8_t>(GPIO_manager::PinMode::OUT));
    }

    SinglePulseOutput::SinglePulseOutput(const JsonVariant &jsonObj, const char* type) : Device(UIDPathMaxLength::One, type) {
        pin = GetAsUINT32(jsonObj, HAL_JSON_KEYNAME_PIN);// jsonObj[HAL_JSON_KEYNAME_PIN];// | 0;//.as<uint8_t>();
        uid = encodeUID(GetAsConstChar(jsonObj, HAL_JSON_KEYNAME_UID));
        //GPIO_manager::ReservePin(pin);
        //const char* uidStr = jsonObj[HAL_JSON_KEYNAME_UID].as<const char*>();
         // jsonObj[HAL_JSON_KEYNAME_UID]);// | "");
        inactiveState = GetAsUINT32(jsonObj, HAL_JSON_KEYNAME_SINGLE_PULSE_OUTPUT_INACTIVE_STATE, 0);
        pulseLength = GetAsUINT32(jsonObj, HAL_JSON_KEYNAME_SINGLE_PULSE_OUTPUT_DEFAULT_PULSE_LENGHT, 0);
        //if (jsonObj.containsKey(HAL_JSON_KEYNAME_SINGLE_PULSE_OUTPUT_INACTIVE_STATE)) {
           // inactiveState = jsonObj[HAL_JSON_KEYNAME_SINGLE_PULSE_OUTPUT_INACTIVE_STATE];// | 0;//.as<uint8_t>();
        //} else {
        //    inactiveState = 0;
        //}
        //if (jsonObj.containsKey(HAL_JSON_KEYNAME_SINGLE_PULSE_OUTPUT_DEFAULT_PULSE_LENGHT)) {
            //pulseLength = jsonObj[HAL_JSON_KEYNAME_SINGLE_PULSE_OUTPUT_DEFAULT_PULSE_LENGHT];// | 0;//.as<uint32_t>();
        //} else {
        //    pulseLength = 0; // will hopefully be set at write
        //}
        pinMode(pin, OUTPUT); // output
        digitalWrite(pin, inactiveState);
    }

    SinglePulseOutput::~SinglePulseOutput() { 
        pinMode(pin, INPUT); // input
        pulseTicker.detach();
    }
#ifndef HAL_JSON_USE_EFFICIENT_FIND
    Device* SinglePulseOutput::findDevice(UIDPath& path) {
        if (path.first() == uid) return this;
        else return nullptr;
    }
#endif
    bool SinglePulseOutput::read(HALValue &val) {
        //val.set(value); // read back the latest write value
        val = pulseLength;
        return true;
    }

    void SinglePulseOutput::pulseTicker_Callback(SinglePulseOutput* context) {
        context->endPulse();
    }

    bool SinglePulseOutput::write(const HALValue &val) {
        uint32_t t = val;
        if (t != 0) // only change if not zero
            pulseLength = t;//val.asUInt();
        if (pulseLength == 0) return true; // no pulse

        digitalWrite(pin, !inactiveState);
        pulseTicker.detach();
        pulseTicker.once_ms(pulseLength, pulseTicker_Callback, this);
        return true;
    }

    void SinglePulseOutput::endPulse() {
        digitalWrite(pin, inactiveState);
    }

    String SinglePulseOutput::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += type;
        ret += "\"";
        ret += DeviceConstStrings::pin;
        ret += std::to_string(pin).c_str();
        ret += ",\"pulseLength\":";
        ret += std::to_string(pulseLength).c_str();
        return ret;
    }

    //  █████  ███    ██  █████  ██       ██████   ██████      ██ ███    ██ ██████  ██    ██ ████████ 
    // ██   ██ ████   ██ ██   ██ ██      ██    ██ ██           ██ ████   ██ ██   ██ ██    ██    ██    
    // ███████ ██ ██  ██ ███████ ██      ██    ██ ██   ███     ██ ██ ██  ██ ██████  ██    ██    ██    
    // ██   ██ ██  ██ ██ ██   ██ ██      ██    ██ ██    ██     ██ ██  ██ ██ ██      ██    ██    ██    
    // ██   ██ ██   ████ ██   ██ ███████  ██████   ██████      ██ ██   ████ ██       ██████     ██    
#if defined(ESP32) || defined(_WIN32)
    Device* AnalogInput::Create(const JsonVariant &jsonObj, const char* type) {
        return new AnalogInput(jsonObj, type);
    }

    bool AnalogInput::VerifyJSON(const JsonVariant &jsonObj) {
        return GPIO_manager::ValidateJsonAndCheckIfPinAvailableAndReserve(jsonObj, static_cast<uint8_t>(GPIO_manager::PinMode::IN));
    }

    AnalogInput::AnalogInput(const JsonVariant &jsonObj, const char* type) : Device(UIDPathMaxLength::One, type) {
        pin = GetAsUINT32(jsonObj, HAL_JSON_KEYNAME_PIN);// jsonObj[HAL_JSON_KEYNAME_PIN];// | 0;//.as<uint8_t>();
        uid = encodeUID(GetAsConstChar(jsonObj, HAL_JSON_KEYNAME_UID)); 
        //pin = jsonObj[HAL_JSON_KEYNAME_PIN];//.as<uint8_t>();
        GPIO_manager::ReservePin(pin);
        //const char* uidStr = jsonObj[HAL_JSON_KEYNAME_UID];//.as<const char*>();
        //uid = encodeUID(uidStr);
        pinMode(pin, INPUT);
    }

    AnalogInput::~AnalogInput() { pinMode(pin, INPUT); } // input
#ifndef HAL_JSON_USE_EFFICIENT_FIND
    Device* AnalogInput::findDevice(UIDPath& path) {
        if (path.first() == uid) return this;
        else return nullptr;
    }
#endif
    bool AnalogInput::read(HALValue &val) {
        //val.set((uint32_t)analogRead(pin));
        val = (uint32_t)analogRead(pin);
        return true;
    }

    String AnalogInput::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += type;
        ret += "\"";
        ret += DeviceConstStrings::pin;
        ret += std::to_string(pin).c_str();
        ret += ",";
        ret += DeviceConstStrings::value;//StartWithComma;
        ret += std::to_string(analogRead(pin)).c_str();
        return ret;
    }
#endif
    // ██████  ██     ██ ███    ███      █████  ███    ██  █████  ██       ██████   ██████      ██     ██ ██████  ██ ████████ ███████      ██████ ███████  ██████  
    // ██   ██ ██     ██ ████  ████     ██   ██ ████   ██ ██   ██ ██      ██    ██ ██           ██     ██ ██   ██ ██    ██    ██          ██      ██      ██       
    // ██████  ██  █  ██ ██ ████ ██     ███████ ██ ██  ██ ███████ ██      ██    ██ ██   ███     ██  █  ██ ██████  ██    ██    █████       ██      █████   ██   ███ 
    // ██      ██ ███ ██ ██  ██  ██     ██   ██ ██  ██ ██ ██   ██ ██      ██    ██ ██    ██     ██ ███ ██ ██   ██ ██    ██    ██          ██      ██      ██    ██ 
    // ██       ███ ███  ██      ██     ██   ██ ██   ████ ██   ██ ███████  ██████   ██████       ███ ███  ██   ██ ██    ██    ███████      ██████ ██       ██████  

    // Define static members somewhere in the cpp file (outside any function)
    uint8_t PWMAnalogWriteConfig::resolution = 0;
    uint32_t PWMAnalogWriteConfig::frequency = 0;

    Device* PWMAnalogWriteConfig::Create(const JsonVariant &jsonObj, const char* type) {
        return new PWMAnalogWriteConfig(jsonObj, type);
    }

    bool PWMAnalogWriteConfig::VerifyJSON(const JsonVariant &jsonObj) {
        if (IsUINT32(jsonObj, HAL_JSON_KEYNAME_PWM_CFG_FREQUENCY) == false) { GlobalLogger.Error(HAL_JSON_ERR_VALUE_TYPE(HAL_JSON_KEYNAME_PWM_CFG_FREQUENCY)); return false; }
        if (IsUINT32(jsonObj, HAL_JSON_KEYNAME_PWM_CFG_RESOLUTION) == false) { GlobalLogger.Error(HAL_JSON_ERR_VALUE_TYPE(HAL_JSON_KEYNAME_PWM_CFG_RESOLUTION)); return false; }
        return true;
    }

    PWMAnalogWriteConfig::PWMAnalogWriteConfig(const JsonVariant &jsonObj, const char* type) : Device(UIDPathMaxLength::One, type) {
        //PWMAnalogWriteConfig::frequency = jsonObj[HAL_JSON_KEYNAME_PWM_CFG_FREQUENCY];//.as<uint32_t>();
        //PWMAnalogWriteConfig::resolution = jsonObj[HAL_JSON_KEYNAME_PWM_CFG_RESOLUTION];//.as<uint32_t>();
        PWMAnalogWriteConfig::frequency = GetAsUINT32(jsonObj, HAL_JSON_KEYNAME_PWM_CFG_FREQUENCY, 0);
        PWMAnalogWriteConfig::resolution = GetAsUINT32(jsonObj, HAL_JSON_KEYNAME_PWM_CFG_RESOLUTION, 0);

#if defined(ESP8266)
        analogWriteResolution(PWMAnalogWriteConfig::resolution);
        analogWriteFreq(PWMAnalogWriteConfig::frequency);
#elif defined(ESP32)        
        analogWriteResolution(PWMAnalogWriteConfig::resolution);
        analogWriteFrequency(PWMAnalogWriteConfig::frequency);
#endif
    }
#ifndef HAL_JSON_USE_EFFICIENT_FIND
    Device* PWMAnalogWriteConfig::findDevice(UIDPath& path) {
        if (path.first() == uid) return this;
        else return nullptr;
    }
#endif
    bool PWMAnalogWriteConfig::write(const HALWriteStringRequestValue& value) {
        return false;
    }

    String PWMAnalogWriteConfig::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += type;
        ret += "\",\"freq\":";
        ret += std::to_string(PWMAnalogWriteConfig::frequency).c_str();
        ret += ",\"resolution\":";
        ret += std::to_string(PWMAnalogWriteConfig::resolution).c_str();
        return ret;
    }

    // ██████  ██     ██ ███    ███      █████  ███    ██  █████  ██       ██████   ██████      ██     ██ ██████  ██ ████████ ███████ 
    // ██   ██ ██     ██ ████  ████     ██   ██ ████   ██ ██   ██ ██      ██    ██ ██           ██     ██ ██   ██ ██    ██    ██      
    // ██████  ██  █  ██ ██ ████ ██     ███████ ██ ██  ██ ███████ ██      ██    ██ ██   ███     ██  █  ██ ██████  ██    ██    █████   
    // ██      ██ ███ ██ ██  ██  ██     ██   ██ ██  ██ ██ ██   ██ ██      ██    ██ ██    ██     ██ ███ ██ ██   ██ ██    ██    ██      
    // ██       ███ ███  ██      ██     ██   ██ ██   ████ ██   ██ ███████  ██████   ██████       ███ ███  ██   ██ ██    ██    ███████ 

    Device* PWMAnalogWrite::Create(const JsonVariant &jsonObj, const char* type) {
        return new PWMAnalogWrite(jsonObj, type);
    }

    bool PWMAnalogWrite::VerifyJSON(const JsonVariant &jsonObj) {
        return GPIO_manager::ValidateJsonAndCheckIfPinAvailableAndReserve(jsonObj, static_cast<uint8_t>(GPIO_manager::PinMode::OUT));
    }

    PWMAnalogWrite::PWMAnalogWrite(const JsonVariant &jsonObj, const char* type) : Device(UIDPathMaxLength::One, type) {
        pin = GetAsUINT32(jsonObj, HAL_JSON_KEYNAME_PIN);// jsonObj[HAL_JSON_KEYNAME_PIN];// | 0;//.as<uint8_t>();
        uid = encodeUID(GetAsConstChar(jsonObj, HAL_JSON_KEYNAME_UID));
        //pin = jsonObj[HAL_JSON_KEYNAME_PIN];//.as<uint8_t>();
        GPIO_manager::ReservePin(pin);
        //const char* uidStr = jsonObj[HAL_JSON_KEYNAME_UID];//.as<const char*>();
        //uid = encodeUID(uidStr);

        pinMode(pin, OUTPUT); // output
    }

    PWMAnalogWrite::~PWMAnalogWrite() { pinMode(pin, INPUT); } // input
#ifndef HAL_JSON_USE_EFFICIENT_FIND
    Device* PWMAnalogWrite::findDevice(UIDPath& path) {
        if (path.first() == uid) return this;
        else return nullptr;
    }
#endif
    bool PWMAnalogWrite::read(HALValue &val) {
        //val.set(value); // just read back latest write
        val = value;
        return true;
    }

    bool PWMAnalogWrite::write(const HALValue &val) {
        //value = val.asUInt();
        value = val;
        if (inv_out)
            value = getInvValue(value);
        analogWrite(pin, value);
        return true;
    }

    String PWMAnalogWrite::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += type;
        ret += "\"";
        ret += DeviceConstStrings::pin;
        ret += std::to_string(pin).c_str();
        ret += ",";
        ret += DeviceConstStrings::value;//StartWithComma;
        ret += std::to_string(value).c_str();
        ret += ",\"inv_out\":";
        ret += std::to_string(inv_out).c_str();
        return ret;
    }

    uint32_t PWMAnalogWrite::getInvValue(uint32_t val)
    {
        if (PWMAnalogWriteConfig::resolution == 10)
            return 1023-val;
        else if (PWMAnalogWriteConfig::resolution == 8)
            return 255-val;

        return 255;
    }
}
