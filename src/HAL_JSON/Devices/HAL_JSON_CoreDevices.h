
#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Ticker.h>
#include "../../Support/Logger.h"
#include "../HAL_JSON_Device.h"
#include "../HAL_JSON_Device_GlobalDefines.h"
#include "../HAL_JSON_ArduinoJSON_ext.h"

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
        static bool VerifyJSON(const JsonVariant &jsonObj);
        static Device* Create(const JsonVariant &jsonObj, const char* type);
        DigitalInput(const JsonVariant &jsonObj, const char* type);
        //~DigitalInput();
#ifndef HAL_JSON_USE_EFFICIENT_FIND
        Device* findDevice(UIDPath& path) override;
#endif
        bool read(HALValue &val) override;
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
        static Device* Create(const JsonVariant &jsonObj, const char* type);
        DigitalOutput(const JsonVariant &jsonObj, const char* type);
        ~DigitalOutput();
#ifndef HAL_JSON_USE_EFFICIENT_FIND
        Device* findDevice(UIDPath& path) override;
#endif
        bool read(HALValue &val) override;
        bool write(const HALValue &val) override;
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
        static Device* Create(const JsonVariant &jsonObj, const char* type);
        SinglePulseOutput(const JsonVariant &jsonObj, const char* type);
        ~SinglePulseOutput();
#ifndef HAL_JSON_USE_EFFICIENT_FIND
        Device* findDevice(UIDPath& path) override;
#endif
        bool read(HALValue &val) override;
        bool write(const HALValue &val) override;
        String ToString() override;
    };


//  █████  ███    ██  █████  ██       ██████   ██████      ██ ███    ██ ██████  ██    ██ ████████ 
// ██   ██ ████   ██ ██   ██ ██      ██    ██ ██           ██ ████   ██ ██   ██ ██    ██    ██    
// ███████ ██ ██  ██ ███████ ██      ██    ██ ██   ███     ██ ██ ██  ██ ██████  ██    ██    ██    
// ██   ██ ██  ██ ██ ██   ██ ██      ██    ██ ██    ██     ██ ██  ██ ██ ██      ██    ██    ██    
// ██   ██ ██   ████ ██   ██ ███████  ██████   ██████      ██ ██   ████ ██       ██████     ██    
#if defined(ESP32)
    class AnalogInput : public Device {
    private:
        uint8_t pin = 0;
    public:
        static bool VerifyJSON(const JsonVariant &jsonObj);
        static Device* Create(const JsonVariant &jsonObj, const char* type);
        AnalogInput(const JsonVariant &jsonObj, const char* type);
        ~AnalogInput();
#ifndef HAL_JSON_USE_EFFICIENT_FIND
        Device* findDevice(UIDPath& path) override;
#endif
        bool read(HALValue &val) override;
        String ToString() override;
    };
#endif
    // TODO implement Analog Input Config to set resolution, but resolution could be individual as it can be set before each read

// ██████  ██     ██ ███    ███      █████  ███    ██  █████  ██       ██████   ██████      ██     ██ ██████  ██ ████████ ███████      ██████ ███████  ██████  
// ██   ██ ██     ██ ████  ████     ██   ██ ████   ██ ██   ██ ██      ██    ██ ██           ██     ██ ██   ██ ██    ██    ██          ██      ██      ██       
// ██████  ██  █  ██ ██ ████ ██     ███████ ██ ██  ██ ███████ ██      ██    ██ ██   ███     ██  █  ██ ██████  ██    ██    █████       ██      █████   ██   ███ 
// ██      ██ ███ ██ ██  ██  ██     ██   ██ ██  ██ ██ ██   ██ ██      ██    ██ ██    ██     ██ ███ ██ ██   ██ ██    ██    ██          ██      ██      ██    ██ 
// ██       ███ ███  ██      ██     ██   ██ ██   ████ ██   ██ ███████  ██████   ██████       ███ ███  ██   ██ ██    ██    ███████      ██████ ██       ██████  

    class PWMAnalogWriteConfig  : public Device { // this do include the base class Device mostly so that the loaded devices can be printed for debug 
    public:
        static bool VerifyJSON(const JsonVariant &jsonObj);
        static Device* Create(const JsonVariant &jsonObj, const char* type);
        static uint8_t resolution; // used together with inv_out to get correct value
        static uint32_t frequency;
        
        PWMAnalogWriteConfig(const JsonVariant &jsonObj, const char* type);
#ifndef HAL_JSON_USE_EFFICIENT_FIND
        Device* findDevice(UIDPath& path) override;
#endif
        bool write(const HALWriteStringRequestValue& value) override;
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
        static Device* Create(const JsonVariant &jsonObj, const char* type);
        PWMAnalogWrite(const JsonVariant &jsonObj, const char* type);
        ~PWMAnalogWrite();
#ifndef HAL_JSON_USE_EFFICIENT_FIND
        Device* findDevice(UIDPath& path) override;
#endif
        bool read(HALValue &val) override;
        bool write(const HALValue &val) override;
        String ToString() override;
    };

    // TODO implement HW_PWM to use with esp32 only (ledc based)
}

