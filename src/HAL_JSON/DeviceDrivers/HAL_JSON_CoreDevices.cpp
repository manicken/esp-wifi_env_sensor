
#include "HAL_JSON_CoreDevices.h"

namespace HAL_JSON {

    
    // ██████  ██  ██████  ██ ████████  █████  ██          ██ ███    ██ ██████  ██    ██ ████████ 
    // ██   ██ ██ ██       ██    ██    ██   ██ ██          ██ ████   ██ ██   ██ ██    ██    ██    
    // ██   ██ ██ ██   ███ ██    ██    ███████ ██          ██ ██ ██  ██ ██████  ██    ██    ██    
    // ██   ██ ██ ██    ██ ██    ██    ██   ██ ██          ██ ██  ██ ██ ██      ██    ██    ██    
    // ██████  ██  ██████  ██    ██    ██   ██ ███████     ██ ██   ████ ██       ██████     ██   

    Device* DigitalInput::Create(const JsonVariant &jsonObj) {
        return new DigitalInput(jsonObj);
    }

    bool DigitalInput::VerifyJSON(const JsonVariant &jsonObj) {
        if (jsonObj.containsKey(HAL_JSON_KEYNAME_PIN) == false) { GlobalLogger.Error(HAL_JSON_ERR_MISSING_KEY(HAL_JSON_KEYNAME_PIN)); return false; }
        if (jsonObj[HAL_JSON_KEYNAME_PIN].is<uint8_t>() == false)  { GlobalLogger.Error(HAL_JSON_ERR_VALUE_TYPE(HAL_JSON_KEYNAME_PIN)); return false; }
        uint8_t pin = jsonObj[HAL_JSON_KEYNAME_PIN].as<uint8_t>();
        return GPIO_manager::CheckIfPinAvailableAndReserve(pin, static_cast<uint8_t>(GPIO_manager::PinMode::IN));
    }

    DigitalInput::DigitalInput(const JsonVariant &jsonObj) : Device(UIDPathMaxLength::One) {
        pin = jsonObj[HAL_JSON_KEYNAME_PIN].as<uint8_t>();
        GPIO_manager::ReservePin(pin);
        
        const char* uidStr = jsonObj[HAL_JSON_KEYNAME_UID].as<const char*>();
        uid = encodeUID(uidStr);

        pinMode(pin, INPUT);
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

    /*bool DigitalInput::write(const HALValue &val req) {  // default is in HAL_JSON_Device
        // read-only, do nothing
        return false;
    }*/

    String DigitalInput::ToString() {
        return "DigitalInput(pin=" + String(pin) + ",val=" + String(digitalRead(pin)) + ")";
    }

    // ██████  ██  ██████  ██ ████████  █████  ██           ██████  ██    ██ ████████ ██████  ██    ██ ████████ 
    // ██   ██ ██ ██       ██    ██    ██   ██ ██          ██    ██ ██    ██    ██    ██   ██ ██    ██    ██    
    // ██   ██ ██ ██   ███ ██    ██    ███████ ██          ██    ██ ██    ██    ██    ██████  ██    ██    ██    
    // ██   ██ ██ ██    ██ ██    ██    ██   ██ ██          ██    ██ ██    ██    ██    ██      ██    ██    ██    
    // ██████  ██  ██████  ██    ██    ██   ██ ███████      ██████   ██████     ██    ██       ██████     ██    
    
    Device* DigitalOutput::Create(const JsonVariant &jsonObj) {
        return new DigitalOutput(jsonObj);
    }

    bool DigitalOutput::VerifyJSON(const JsonVariant &jsonObj) {
        if (jsonObj.containsKey(HAL_JSON_KEYNAME_PIN) == false) { GlobalLogger.Error(HAL_JSON_ERR_MISSING_KEY(HAL_JSON_KEYNAME_PIN)); return false; }
        if (jsonObj[HAL_JSON_KEYNAME_PIN].is<uint8_t>() == false)  { GlobalLogger.Error(HAL_JSON_ERR_VALUE_TYPE(HAL_JSON_KEYNAME_PIN)); return false; }
        uint8_t pin = jsonObj[HAL_JSON_KEYNAME_PIN].as<uint8_t>(); 
        return GPIO_manager::CheckIfPinAvailableAndReserve(pin, static_cast<uint8_t>(GPIO_manager::PinMode::OUT));
    }

    DigitalOutput::DigitalOutput(const JsonVariant &jsonObj) : Device(UIDPathMaxLength::One) {
        pin = jsonObj[HAL_JSON_KEYNAME_PIN].as<uint8_t>();
        GPIO_manager::ReservePin(pin);

        const char* uidStr = jsonObj[HAL_JSON_KEYNAME_UID].as<const char*>();
        uid = encodeUID(uidStr);

        pinMode(pin, OUTPUT);
    }

    DigitalOutput::~DigitalOutput() { pinMode(pin, INPUT); } // release the pin
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
        return "DigitalOutput(pin=" +  String(pin) + ",val=" + String(value) +  + ")";
    }

    // ██████  ██    ██ ██      ███████ ███████      ██████  ██    ██ ████████ ██████  ██    ██ ████████ 
    // ██   ██ ██    ██ ██      ██      ██          ██    ██ ██    ██    ██    ██   ██ ██    ██    ██    
    // ██████  ██    ██ ██      ███████ █████       ██    ██ ██    ██    ██    ██████  ██    ██    ██    
    // ██      ██    ██ ██           ██ ██          ██    ██ ██    ██    ██    ██      ██    ██    ██    
    // ██       ██████  ███████ ███████ ███████      ██████   ██████     ██    ██       ██████     ██    

    Device* SinglePulseOutput::Create(const JsonVariant &jsonObj) {
        return new SinglePulseOutput(jsonObj);
    }

    bool SinglePulseOutput::VerifyJSON(const JsonVariant &jsonObj) {
        if (jsonObj.containsKey(HAL_JSON_KEYNAME_PIN) == false) { GlobalLogger.Error(HAL_JSON_ERR_MISSING_KEY(HAL_JSON_KEYNAME_PIN)); return false; }
        if (jsonObj[HAL_JSON_KEYNAME_PIN].is<uint8_t>() == false)  { GlobalLogger.Error(HAL_JSON_ERR_VALUE_TYPE(HAL_JSON_KEYNAME_PIN)); return false; }
        uint8_t pin = jsonObj[HAL_JSON_KEYNAME_PIN].as<uint8_t>(); 
        return GPIO_manager::CheckIfPinAvailableAndReserve(pin, static_cast<uint8_t>(GPIO_manager::PinMode::OUT));
    }

    SinglePulseOutput::SinglePulseOutput(const JsonVariant &jsonObj) : Device(UIDPathMaxLength::One) {
        pin = jsonObj[HAL_JSON_KEYNAME_PIN].as<uint8_t>();
        GPIO_manager::ReservePin(pin);
        const char* uidStr = jsonObj[HAL_JSON_KEYNAME_UID].as<const char*>();
        uid = encodeUID(uidStr);

        if (jsonObj.containsKey(HAL_JSON_KEYNAME_SINGLE_PULSE_OUTPUT_INACTIVE_STATE)) {
            inactiveState = jsonObj[HAL_JSON_KEYNAME_SINGLE_PULSE_OUTPUT_INACTIVE_STATE].as<uint8_t>();
        } else {
            inactiveState = 0;
        }
        if (jsonObj.containsKey(HAL_JSON_KEYNAME_SINGLE_PULSE_OUTPUT_DEFAULT_PULSE_LENGHT)) {
            pulseLength = jsonObj[HAL_JSON_KEYNAME_SINGLE_PULSE_OUTPUT_DEFAULT_PULSE_LENGHT].as<uint32_t>();
        } else {
            pulseLength = 0; // will hopefully be set at write
        }
        pinMode(pin, OUTPUT);
        digitalWrite(pin, inactiveState);
    }

    SinglePulseOutput::~SinglePulseOutput() { 
        pinMode(pin, INPUT);
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
        return "SinglePulseOutput(pin=" +  String(pin) + ",val=" + String(pulseLength) +  + ")";
    }

    //  █████  ███    ██  █████  ██       ██████   ██████      ██ ███    ██ ██████  ██    ██ ████████ 
    // ██   ██ ████   ██ ██   ██ ██      ██    ██ ██           ██ ████   ██ ██   ██ ██    ██    ██    
    // ███████ ██ ██  ██ ███████ ██      ██    ██ ██   ███     ██ ██ ██  ██ ██████  ██    ██    ██    
    // ██   ██ ██  ██ ██ ██   ██ ██      ██    ██ ██    ██     ██ ██  ██ ██ ██      ██    ██    ██    
    // ██   ██ ██   ████ ██   ██ ███████  ██████   ██████      ██ ██   ████ ██       ██████     ██    

    Device* AnalogInput::Create(const JsonVariant &jsonObj) {
        return new AnalogInput(jsonObj);
    }

    bool AnalogInput::VerifyJSON(const JsonVariant &jsonObj) {
        if (jsonObj.containsKey(HAL_JSON_KEYNAME_PIN) == false) { GlobalLogger.Error(HAL_JSON_ERR_MISSING_KEY(HAL_JSON_KEYNAME_PIN)); return false; }
        if (jsonObj[HAL_JSON_KEYNAME_PIN].is<uint8_t>() == false)  { GlobalLogger.Error(HAL_JSON_ERR_VALUE_TYPE(HAL_JSON_KEYNAME_PIN)); return false; }
        uint8_t pin = jsonObj[HAL_JSON_KEYNAME_PIN].as<uint8_t>(); 
        return GPIO_manager::CheckIfPinAvailableAndReserve(pin, static_cast<uint8_t>(GPIO_manager::PinMode::IN));
    }

    AnalogInput::AnalogInput(const JsonVariant &jsonObj) : Device(UIDPathMaxLength::One) {
        pin = jsonObj[HAL_JSON_KEYNAME_PIN].as<uint8_t>();
        GPIO_manager::ReservePin(pin);
        const char* uidStr = jsonObj[HAL_JSON_KEYNAME_UID].as<const char*>();
        uid = encodeUID(uidStr);
        pinMode(pin, ANALOG);
    }

    AnalogInput::~AnalogInput() { pinMode(pin, INPUT); }
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

    /*bool AnalogInput::write(const HALValue &val req) {  // default is in HAL_JSON_Device
        // read-only, do nothing
        return false;
    }*/

    String AnalogInput::ToString() {
        return "AnalogInput(pin=" +  String(pin) + ",val=" + String(analogRead(pin)) + ")";
    }

    // ██████  ██     ██ ███    ███      █████  ███    ██  █████  ██       ██████   ██████      ██     ██ ██████  ██ ████████ ███████      ██████ ███████  ██████  
    // ██   ██ ██     ██ ████  ████     ██   ██ ████   ██ ██   ██ ██      ██    ██ ██           ██     ██ ██   ██ ██    ██    ██          ██      ██      ██       
    // ██████  ██  █  ██ ██ ████ ██     ███████ ██ ██  ██ ███████ ██      ██    ██ ██   ███     ██  █  ██ ██████  ██    ██    █████       ██      █████   ██   ███ 
    // ██      ██ ███ ██ ██  ██  ██     ██   ██ ██  ██ ██ ██   ██ ██      ██    ██ ██    ██     ██ ███ ██ ██   ██ ██    ██    ██          ██      ██      ██    ██ 
    // ██       ███ ███  ██      ██     ██   ██ ██   ████ ██   ██ ███████  ██████   ██████       ███ ███  ██   ██ ██    ██    ███████      ██████ ██       ██████  

    Device* PWMAnalogWriteConfig::Create(const JsonVariant &jsonObj) {
        return new PWMAnalogWriteConfig(jsonObj);
    }

    bool PWMAnalogWriteConfig::VerifyJSON(const JsonVariant &jsonObj) {
        if (jsonObj[HAL_JSON_KEYNAME_PWM_CFG_FREQUENCY].is<uint32_t>() == false) { GlobalLogger.Error(HAL_JSON_ERR_VALUE_TYPE(HAL_JSON_KEYNAME_PWM_CFG_FREQUENCY)); return false; }
        if (jsonObj[HAL_JSON_KEYNAME_PWM_CFG_RESOLUTION].is<uint32_t>() == false) { GlobalLogger.Error(HAL_JSON_ERR_VALUE_TYPE(HAL_JSON_KEYNAME_PWM_CFG_RESOLUTION)); return false; }
        return true;
    }

    PWMAnalogWriteConfig::PWMAnalogWriteConfig(const JsonVariant &jsonObj) : Device(UIDPathMaxLength::One) {
        PWMAnalogWriteConfig::frequency = jsonObj[HAL_JSON_KEYNAME_PWM_CFG_FREQUENCY].as<uint32_t>();
        PWMAnalogWriteConfig::resolution = jsonObj[HAL_JSON_KEYNAME_PWM_CFG_RESOLUTION].as<uint32_t>();

#if defined(ESP8266)
        analogWriteResolution(PWMAnalogWriteConfig::resolution);
        analogWriteFreq(frequPWMAnalogWriteConfig::frequencyency);
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

    //bool PWMAnalogWriteConfig::read(HALReadRequest &req) { return false; } // default is in HAL_JSON_Device
    //bool PWMAnalogWriteConfig::write(const HALValue &val req) { return false; } // default is in HAL_JSON_Device
    String PWMAnalogWriteConfig::ToString() {
        return "PWMAnalogWriteConfig(freq=" + String(PWMAnalogWriteConfig::frequency) + ", resolution=" + String(PWMAnalogWriteConfig::resolution) + ")";
    }

    // ██████  ██     ██ ███    ███      █████  ███    ██  █████  ██       ██████   ██████      ██     ██ ██████  ██ ████████ ███████ 
    // ██   ██ ██     ██ ████  ████     ██   ██ ████   ██ ██   ██ ██      ██    ██ ██           ██     ██ ██   ██ ██    ██    ██      
    // ██████  ██  █  ██ ██ ████ ██     ███████ ██ ██  ██ ███████ ██      ██    ██ ██   ███     ██  █  ██ ██████  ██    ██    █████   
    // ██      ██ ███ ██ ██  ██  ██     ██   ██ ██  ██ ██ ██   ██ ██      ██    ██ ██    ██     ██ ███ ██ ██   ██ ██    ██    ██      
    // ██       ███ ███  ██      ██     ██   ██ ██   ████ ██   ██ ███████  ██████   ██████       ███ ███  ██   ██ ██    ██    ███████ 

    Device* PWMAnalogWrite::Create(const JsonVariant &jsonObj) {
        return new PWMAnalogWrite(jsonObj);
    }

    bool PWMAnalogWrite::VerifyJSON(const JsonVariant &jsonObj) {
        if (jsonObj.containsKey(HAL_JSON_KEYNAME_PIN) == false) { GlobalLogger.Error(HAL_JSON_ERR_MISSING_KEY(HAL_JSON_KEYNAME_PIN)); return false; }
        if (jsonObj[HAL_JSON_KEYNAME_PIN].is<uint8_t>() == false)  { GlobalLogger.Error(HAL_JSON_ERR_VALUE_TYPE(HAL_JSON_KEYNAME_PIN)); return false; }
        uint8_t pin = jsonObj[HAL_JSON_KEYNAME_PIN].as<uint8_t>(); 
        return GPIO_manager::CheckIfPinAvailable(pin, static_cast<uint8_t>(GPIO_manager::PinMode::OUT));
    }

    PWMAnalogWrite::PWMAnalogWrite(const JsonVariant &jsonObj) : Device(UIDPathMaxLength::One) {
        pin = jsonObj[HAL_JSON_KEYNAME_PIN].as<uint8_t>();
        GPIO_manager::ReservePin(pin);
        const char* uidStr = jsonObj[HAL_JSON_KEYNAME_UID].as<const char*>();
        uid = encodeUID(uidStr);

        pinMode(pin, OUTPUT);
    }

    PWMAnalogWrite::~PWMAnalogWrite() { pinMode(pin, INPUT); }
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
        return "PWMAnalogWrite(pin=" +  String(pin) + ", val=" + String(value) + ", inv_out=" + String(inv_out) + ")";
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
