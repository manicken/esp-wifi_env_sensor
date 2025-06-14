#pragma once

#include <Arduino.h>
#include <Ticker.h>
#include "HALDevice.h" // assuming this contains HalDevice

// ASCII ART generated here:
// https://patorjk.com/software/taag/#p=display&f=ANSI%20Regular&t=ASCII%20ART

namespace HAL {

// ██████  ██  ██████  ██ ████████  █████  ██          ██ ███    ██ ██████  ██    ██ ████████ 
// ██   ██ ██ ██       ██    ██    ██   ██ ██          ██ ████   ██ ██   ██ ██    ██    ██    
// ██   ██ ██ ██   ███ ██    ██    ███████ ██          ██ ██ ██  ██ ██████  ██    ██    ██    
// ██   ██ ██ ██    ██ ██    ██    ██   ██ ██          ██ ██  ██ ██ ██      ██    ██    ██    
// ██████  ██  ██████  ██    ██    ██   ██ ███████     ██ ██   ████ ██       ██████     ██  

    class DigitalInput : public Device {
    private:
        uint8_t pin = 0;
    public:
        DigitalInput(uint8_t pin);
        //~DigitalInput();
        bool read(HALValue&) override;
        bool write(const HALValue&) override;
        String ToString() override;
    };

// ██████  ██  ██████  ██ ████████  █████  ██           ██████  ██    ██ ████████ ██████  ██    ██ ████████ 
// ██   ██ ██ ██       ██    ██    ██   ██ ██          ██    ██ ██    ██    ██    ██   ██ ██    ██    ██    
// ██   ██ ██ ██   ███ ██    ██    ███████ ██          ██    ██ ██    ██    ██    ██████  ██    ██    ██    
// ██   ██ ██ ██    ██ ██    ██    ██   ██ ██          ██    ██ ██    ██    ██    ██      ██    ██    ██    
// ██████  ██  ██████  ██    ██    ██   ██ ███████      ██████   ██████     ██    ██       ██████     ██    

    class DigitalOutput : public Device {
    private:
        uint8_t pin = 0;
        uint32_t value = 0;
    public:
        DigitalOutput(uint8_t pin);
        ~DigitalOutput();
        bool read(HALValue&) override;
        bool write(const HALValue&) override;
        String ToString() override;
    };

// ███████ ██ ███    ██  ██████  ██      ███████     ██████  ██    ██ ██      ███████ ███████      ██████  ██    ██ ████████ ██████  ██    ██ ████████ 
// ██      ██ ████   ██ ██       ██      ██          ██   ██ ██    ██ ██      ██      ██          ██    ██ ██    ██    ██    ██   ██ ██    ██    ██    
// ███████ ██ ██ ██  ██ ██   ███ ██      █████       ██████  ██    ██ ██      ███████ █████       ██    ██ ██    ██    ██    ██████  ██    ██    ██    
//      ██ ██ ██  ██ ██ ██    ██ ██      ██          ██      ██    ██ ██           ██ ██          ██    ██ ██    ██    ██    ██      ██    ██    ██    
// ███████ ██ ██   ████  ██████  ███████ ███████     ██       ██████  ███████ ███████ ███████      ██████   ██████     ██    ██       ██████     ██    
                                                                                                                                                    
    class SinglePulseOutput : public Device {
    private:
        uint8_t pin = 0;
        uint32_t value = 0;
        uint8_t inactiveState = 0;
        Ticker pulseTicker;
        void endPulse();
        static void pulseTicker_Callback(SinglePulseOutput* context);
    public:
        SinglePulseOutput(uint8_t _pin, uint8_t _inactiveState = LOW);
        ~SinglePulseOutput();
        bool read(HALValue&) override;
        bool write(const HALValue&) override;
        String ToString() override;
    };


//  █████  ███    ██  █████  ██       ██████   ██████      ██ ███    ██ ██████  ██    ██ ████████ 
// ██   ██ ████   ██ ██   ██ ██      ██    ██ ██           ██ ████   ██ ██   ██ ██    ██    ██    
// ███████ ██ ██  ██ ███████ ██      ██    ██ ██   ███     ██ ██ ██  ██ ██████  ██    ██    ██    
// ██   ██ ██  ██ ██ ██   ██ ██      ██    ██ ██    ██     ██ ██  ██ ██ ██      ██    ██    ██    
// ██   ██ ██   ████ ██   ██ ███████  ██████   ██████      ██ ██   ████ ██       ██████     ██    

    class AnalogInput : public Device {
    private:
        uint8_t pin = 0;
    public:
        AnalogInput(uint8_t pin);
        ~AnalogInput();
        bool read(HALValue&) override;
        bool write(const HALValue&) override;
        String ToString() override;
    };

    // TODO implement Analog Input Config to set resolution, but resolution could be individual as it can be set before each read

// ██████  ██     ██ ███    ███      █████  ███    ██  █████  ██       ██████   ██████      ██     ██ ██████  ██ ████████ ███████      ██████ ███████  ██████  
// ██   ██ ██     ██ ████  ████     ██   ██ ████   ██ ██   ██ ██      ██    ██ ██           ██     ██ ██   ██ ██    ██    ██          ██      ██      ██       
// ██████  ██  █  ██ ██ ████ ██     ███████ ██ ██  ██ ███████ ██      ██    ██ ██   ███     ██  █  ██ ██████  ██    ██    █████       ██      █████   ██   ███ 
// ██      ██ ███ ██ ██  ██  ██     ██   ██ ██  ██ ██ ██   ██ ██      ██    ██ ██    ██     ██ ███ ██ ██   ██ ██    ██    ██          ██      ██      ██    ██ 
// ██       ███ ███  ██      ██     ██   ██ ██   ████ ██   ██ ███████  ██████   ██████       ███ ███  ██   ██ ██    ██    ███████      ██████ ██       ██████  

    class PWMAnalogWriteConfig  : public Device { // this do include the base class Device mostly so that the loaded devices can be printed for debug 
    public:
        static uint8_t resolution; // used together with inv_out to get correct value
        static uint32_t frequency;
        
        PWMAnalogWriteConfig(uint32_t frequency, uint8_t resolution);
        bool read(HALValue&) override;
        bool write(const HALValue&) override;
        String ToString() override;

    };

// ██████  ██     ██ ███    ███      █████  ███    ██  █████  ██       ██████   ██████      ██     ██ ██████  ██ ████████ ███████ 
// ██   ██ ██     ██ ████  ████     ██   ██ ████   ██ ██   ██ ██      ██    ██ ██           ██     ██ ██   ██ ██    ██    ██      
// ██████  ██  █  ██ ██ ████ ██     ███████ ██ ██  ██ ███████ ██      ██    ██ ██   ███     ██  █  ██ ██████  ██    ██    █████   
// ██      ██ ███ ██ ██  ██  ██     ██   ██ ██  ██ ██ ██   ██ ██      ██    ██ ██    ██     ██ ███ ██ ██   ██ ██    ██    ██      
// ██       ███ ███  ██      ██     ██   ██ ██   ████ ██   ██ ███████  ██████   ██████       ███ ███  ██   ██ ██    ██    ███████ 

    class PWMAnalogWrite : public Device {
    private:
        uint8_t pin = 0;
        uint8_t inv_out = 0;
        uint32_t value = 0; // for readback only
        uint32_t getInvValue(uint32_t val);
    public:
        PWMAnalogWrite(uint8_t pin, uint8_t inv_out);
        ~PWMAnalogWrite();
        bool read(HALValue&) override;
        bool write(const HALValue&) override;
        String ToString() override;
    };

    // TODO implement HW_PWM to use with esp32 only (ledc based)
}

