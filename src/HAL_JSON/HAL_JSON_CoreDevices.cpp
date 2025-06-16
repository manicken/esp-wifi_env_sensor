
#include "HAL_JSON_CoreDevices.h"

namespace HAL_JSON {

    
// ██████  ██  ██████  ██ ████████  █████  ██          ██ ███    ██ ██████  ██    ██ ████████ 
// ██   ██ ██ ██       ██    ██    ██   ██ ██          ██ ████   ██ ██   ██ ██    ██    ██    
// ██   ██ ██ ██   ███ ██    ██    ███████ ██          ██ ██ ██  ██ ██████  ██    ██    ██    
// ██   ██ ██ ██    ██ ██    ██    ██   ██ ██          ██ ██  ██ ██ ██      ██    ██    ██    
// ██████  ██  ██████  ██    ██    ██   ██ ███████     ██ ██   ████ ██       ██████     ██   

    DigitalInput::DigitalInput(uint8_t pin) : pin(pin){ pinMode(pin, INPUT); }

    bool DigitalInput::read(HALValue &val) {
        //val.set((uint32_t)digitalRead(pin));
        val = (uint32_t)digitalRead(pin);
        return true;
    }

    bool DigitalInput::write(const HALValue& val) {
        // read-only, do nothing
        return false;
    }

    String DigitalInput::ToString() {
        return "DigitalInput(pin=" + String(pin) + ",val=" + String(digitalRead(pin)) + ")";
    }

// ██████  ██  ██████  ██ ████████  █████  ██           ██████  ██    ██ ████████ ██████  ██    ██ ████████ 
// ██   ██ ██ ██       ██    ██    ██   ██ ██          ██    ██ ██    ██    ██    ██   ██ ██    ██    ██    
// ██   ██ ██ ██   ███ ██    ██    ███████ ██          ██    ██ ██    ██    ██    ██████  ██    ██    ██    
// ██   ██ ██ ██    ██ ██    ██    ██   ██ ██          ██    ██ ██    ██    ██    ██      ██    ██    ██    
// ██████  ██  ██████  ██    ██    ██   ██ ███████      ██████   ██████     ██    ██       ██████     ██    
                                                                                                         
    DigitalOutput::DigitalOutput(uint8_t pin) : pin(pin){ pinMode(pin, OUTPUT); }
    DigitalOutput::~DigitalOutput() { pinMode(pin, INPUT); } // release the pin

    bool DigitalOutput::read(HALValue &val) {
        //val.set(value); // read back the latest write value
        val = value;
        return true;
    }

    bool DigitalOutput::write(const HALValue& val) {
        value = val;//val.asUInt();
        digitalWrite(pin, val);
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

    SinglePulseOutput::SinglePulseOutput(uint8_t _pin, uint8_t _inactiveState) : pin(_pin),inactiveState(_inactiveState) {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, inactiveState);
    }
    SinglePulseOutput::~SinglePulseOutput() { 
        pinMode(pin, INPUT);
        pulseTicker.detach();
    }

    bool SinglePulseOutput::read(HALValue &val) {
        //val.set(value); // read back the latest write value
        val = value;
        return true;
    }

    void SinglePulseOutput::pulseTicker_Callback(SinglePulseOutput* context) {
        context->endPulse();
    }

    bool SinglePulseOutput::write(const HALValue& val) {
        value = val;//val.asUInt();
        digitalWrite(pin, !inactiveState);
        pulseTicker.detach();
        pulseTicker.once_ms(value, pulseTicker_Callback, this);
        return true;
    }

    void SinglePulseOutput::endPulse() {
        digitalWrite(pin, inactiveState);
    }

    String SinglePulseOutput::ToString() {
        return "SinglePulseOutput(pin=" +  String(pin) + ",val=" + String(value) +  + ")";
    }

//  █████  ███    ██  █████  ██       ██████   ██████      ██ ███    ██ ██████  ██    ██ ████████ 
// ██   ██ ████   ██ ██   ██ ██      ██    ██ ██           ██ ████   ██ ██   ██ ██    ██    ██    
// ███████ ██ ██  ██ ███████ ██      ██    ██ ██   ███     ██ ██ ██  ██ ██████  ██    ██    ██    
// ██   ██ ██  ██ ██ ██   ██ ██      ██    ██ ██    ██     ██ ██  ██ ██ ██      ██    ██    ██    
// ██   ██ ██   ████ ██   ██ ███████  ██████   ██████      ██ ██   ████ ██       ██████     ██    

    AnalogInput::AnalogInput(uint8_t pin) : pin(pin){ pinMode(pin, ANALOG); }
    AnalogInput::~AnalogInput() { pinMode(pin, INPUT); }


    bool AnalogInput::read(HALValue &val) {
        //val.set((uint32_t)analogRead(pin));
        val = (uint32_t)analogRead(pin);
        return true;
    }

    bool AnalogInput::write(const HALValue& val) {
        // read-only, do nothing
        return false;
    }

    String AnalogInput::ToString() {
        return "AnalogInput(pin=" +  String(pin) + ",val=" + String(analogRead(pin)) + ")";
    }

// ██████  ██     ██ ███    ███      █████  ███    ██  █████  ██       ██████   ██████      ██     ██ ██████  ██ ████████ ███████      ██████ ███████  ██████  
// ██   ██ ██     ██ ████  ████     ██   ██ ████   ██ ██   ██ ██      ██    ██ ██           ██     ██ ██   ██ ██    ██    ██          ██      ██      ██       
// ██████  ██  █  ██ ██ ████ ██     ███████ ██ ██  ██ ███████ ██      ██    ██ ██   ███     ██  █  ██ ██████  ██    ██    █████       ██      █████   ██   ███ 
// ██      ██ ███ ██ ██  ██  ██     ██   ██ ██  ██ ██ ██   ██ ██      ██    ██ ██    ██     ██ ███ ██ ██   ██ ██    ██    ██          ██      ██      ██    ██ 
// ██       ███ ███  ██      ██     ██   ██ ██   ████ ██   ██ ███████  ██████   ██████       ███ ███  ██   ██ ██    ██    ███████      ██████ ██       ██████  

    PWMAnalogWriteConfig::PWMAnalogWriteConfig(uint32_t frequency, uint8_t resolution) {
        PWMAnalogWriteConfig::frequency = frequency;
        PWMAnalogWriteConfig::resolution = resolution;

#if defined(ESP8266)
        analogWriteResolution(resolution);
        analogWriteFreq(frequency);
#elif defined(ESP32)        
        analogWriteResolution(resolution);
        analogWriteFrequency(frequency);
#endif
    }
    bool PWMAnalogWriteConfig::read(HALValue &val) { return false; }
    bool PWMAnalogWriteConfig::write(const HALValue& val) { return false; }
    String PWMAnalogWriteConfig::ToString() {
        return "PWMAnalogWriteConfig(freq=" + String(PWMAnalogWriteConfig::frequency) + ", resolution=" + String(PWMAnalogWriteConfig::resolution) + ")";
    }

// ██████  ██     ██ ███    ███      █████  ███    ██  █████  ██       ██████   ██████      ██     ██ ██████  ██ ████████ ███████ 
// ██   ██ ██     ██ ████  ████     ██   ██ ████   ██ ██   ██ ██      ██    ██ ██           ██     ██ ██   ██ ██    ██    ██      
// ██████  ██  █  ██ ██ ████ ██     ███████ ██ ██  ██ ███████ ██      ██    ██ ██   ███     ██  █  ██ ██████  ██    ██    █████   
// ██      ██ ███ ██ ██  ██  ██     ██   ██ ██  ██ ██ ██   ██ ██      ██    ██ ██    ██     ██ ███ ██ ██   ██ ██    ██    ██      
// ██       ███ ███  ██      ██     ██   ██ ██   ████ ██   ██ ███████  ██████   ██████       ███ ███  ██   ██ ██    ██    ███████ 

    PWMAnalogWrite::PWMAnalogWrite(uint8_t pin, uint8_t inv_out) : pin(pin),inv_out(inv_out) {
        pinMode(pin, OUTPUT);
    }
    PWMAnalogWrite::~PWMAnalogWrite() { pinMode(pin, INPUT); }

    bool PWMAnalogWrite::read(HALValue &val) {
        //val.set(value); // just read back latest write
        val = value;
        return true;
    }

    bool PWMAnalogWrite::write(const HALValue& val) {
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
