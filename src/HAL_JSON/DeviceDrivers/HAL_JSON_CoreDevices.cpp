
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

    DigitalInput::DigitalInput(const JsonVariant &jsonObj) {
        pin = jsonObj[HAL_JSON_KEYNAME_PIN].as<uint8_t>();
        // pin is reserved in ValidateJSON
        
        const char* uidStr = jsonObj[HAL_JSON_KEYNAME_UID].as<const char*>();
        uid = encodeUID(uidStr);

        pinMode(pin, INPUT);
    }

    bool DigitalInput::read(const HALReadRequest &req) {
        //val.set((uint32_t)digitalRead(pin));
        req.out_value = (uint32_t)digitalRead(pin);
        return true;
    }

    /*bool DigitalInput::write(const HALWriteRequest& req) {  // default is in HAL_JSON_Device
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

    DigitalOutput::DigitalOutput(const JsonVariant &jsonObj) {
        pin = jsonObj[HAL_JSON_KEYNAME_PIN].as<uint8_t>();
        // pin is reserved in ValidateJSON

        const char* uidStr = jsonObj[HAL_JSON_KEYNAME_UID].as<const char*>();
        uid = encodeUID(uidStr);

        pinMode(pin, OUTPUT);
    }

    DigitalOutput::~DigitalOutput() { pinMode(pin, INPUT); } // release the pin

    bool DigitalOutput::read(const HALReadRequest &req) {
        //val.set(value); // read back the latest write value
        req.out_value = value;
        return true;
    }

    bool DigitalOutput::write(const HALWriteRequest& req) {
        value = req.value;//val.asUInt();
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

    SinglePulseOutput::SinglePulseOutput(const JsonVariant &jsonObj) {
        pin = jsonObj[HAL_JSON_KEYNAME_PIN].as<uint8_t>();
        // pin is reserved in ValidateJSON
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

    bool SinglePulseOutput::read(const HALReadRequest &req) {
        //val.set(value); // read back the latest write value
        req.out_value = pulseLength;
        return true;
    }

    void SinglePulseOutput::pulseTicker_Callback(SinglePulseOutput* context) {
        context->endPulse();
    }

    bool SinglePulseOutput::write(const HALWriteRequest& req) {
        uint32_t t = req.value;
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

    AnalogInput::AnalogInput(const JsonVariant &jsonObj) {
        pin = jsonObj[HAL_JSON_KEYNAME_PIN].as<uint8_t>();
        // pin is reserved in ValidateJSON
        const char* uidStr = jsonObj[HAL_JSON_KEYNAME_UID].as<const char*>();
        uid = encodeUID(uidStr);
        pinMode(pin, ANALOG);
    }

    AnalogInput::~AnalogInput() { pinMode(pin, INPUT); }

    bool AnalogInput::read(const HALReadRequest &req) {
        //val.set((uint32_t)analogRead(pin));
        req.out_value = (uint32_t)analogRead(pin);
        return true;
    }

    /*bool AnalogInput::write(const HALWriteRequest& req) {  // default is in HAL_JSON_Device
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

    PWMAnalogWriteConfig::PWMAnalogWriteConfig(const JsonVariant &jsonObj) {
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

    //bool PWMAnalogWriteConfig::read(HALReadRequest &req) { return false; } // default is in HAL_JSON_Device
    //bool PWMAnalogWriteConfig::write(const HALWriteRequest& req) { return false; } // default is in HAL_JSON_Device
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

    PWMAnalogWrite::PWMAnalogWrite(const JsonVariant &jsonObj) {
        pin = jsonObj[HAL_JSON_KEYNAME_PIN].as<uint8_t>();
        // pin is reserved in ValidateJSON
        const char* uidStr = jsonObj[HAL_JSON_KEYNAME_UID].as<const char*>();
        uid = encodeUID(uidStr);

        pinMode(pin, OUTPUT);
    }

    PWMAnalogWrite::~PWMAnalogWrite() { pinMode(pin, INPUT); }

    bool PWMAnalogWrite::read(const HALReadRequest &req) {
        //val.set(value); // just read back latest write
        req.out_value = value;
        return true;
    }

    bool PWMAnalogWrite::write(const HALWriteRequest& req) {
        //value = val.asUInt();
        value = req.value;
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
