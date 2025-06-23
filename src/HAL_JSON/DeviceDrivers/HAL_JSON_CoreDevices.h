#pragma once

#include <Arduino.h>
#include <Ticker.h>
#include "../../Support/Logger.h"
#include "../HAL_JSON_Device.h"
#include "../HAL_JSON_DeviceTypeDefNames.h"

// ASCII ART generated here:
// https://patorjk.com/software/taag/#p=display&f=ANSI%20Regular&t=ASCII%20ART

namespace HAL_JSON {

// ██████  ██  ██████  ██ ████████  █████  ██          ██ ███    ██ ██████  ██    ██ ████████ 
// ██   ██ ██ ██       ██    ██    ██   ██ ██          ██ ████   ██ ██   ██ ██    ██    ██    
// ██   ██ ██ ██   ███ ██    ██    ███████ ██          ██ ██ ██  ██ ██████  ██    ██    ██    
// ██   ██ ██ ██    ██ ██    ██    ██   ██ ██          ██ ██  ██ ██ ██      ██    ██    ██    
// ██████  ██  ██████  ██    ██    ██   ██ ███████     ██ ██   ████ ██       ██████     ██  

    class DigitalInput : public Device {
    private:
        uint8_t pin = 0;
    public:
        static bool VerifyJSON(const JsonVariant &json);
        static Device* Create(const JsonVariant &json);
        DigitalInput(const JsonVariant &jsonObj);
        //~DigitalInput();
        bool read(const HALReadRequest&) override;
        bool write(const HALWriteRequest&) override;
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
        static bool VerifyJSON(const JsonVariant &jsonObj);
        static Device* Create(const JsonVariant &jsonObj);
        DigitalOutput(const JsonVariant &jsonObj);
        ~DigitalOutput();
        bool read(const HALReadRequest&) override;
        bool write(const HALWriteRequest&) override;
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
        uint32_t pulseLength = 0;
        uint8_t inactiveState = 0;
        Ticker pulseTicker;
        void endPulse();
        static void pulseTicker_Callback(SinglePulseOutput* context);
    public:
        static bool VerifyJSON(const JsonVariant &jsonObj);
        static Device* Create(const JsonVariant &jsonObj);
        SinglePulseOutput(const JsonVariant &jsonObj);
        ~SinglePulseOutput();
        bool read(const HALReadRequest&) override;
        bool write(const HALWriteRequest&) override;
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
        static bool VerifyJSON(const JsonVariant &jsonObj);
        static Device* Create(const JsonVariant &jsonObj);
        AnalogInput(const JsonVariant &jsonObj);
        ~AnalogInput();
        bool read(const HALReadRequest&) override;
        bool write(const HALWriteRequest&) override;
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
        static bool VerifyJSON(const JsonVariant &jsonObj);
        static Device* Create(const JsonVariant &jsonObj);
        static uint8_t resolution; // used together with inv_out to get correct value
        static uint32_t frequency;
        
        PWMAnalogWriteConfig(const JsonVariant &jsonObj);
        bool read(const HALReadRequest&) override;
        bool write(const HALWriteRequest&) override;
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
        static bool VerifyJSON(const JsonVariant &jsonObj);
        static Device* Create(const JsonVariant &jsonObj);
        PWMAnalogWrite(const JsonVariant &jsonObj);
        ~PWMAnalogWrite();
        bool read(const HALReadRequest&) override;
        bool write(const HALWriteRequest&) override;
        String ToString() override;
    };

    // TODO implement HW_PWM to use with esp32 only (ledc based)
}

