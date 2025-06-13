
#pragma once

#include <Arduino.h>

namespace HAL {

    /*enum class DeviceType : int32_t
    {
        Unknown = -1, //static_cast<int>(0xFFFFFFFF),
        OneWireBus = 0x1B,
        OneWireTemp = 0x10,
        DHT = 0x444854, // ascii hex for DHT
        PWM = 0xF,
        TX433 = 0x433,
        ADC = 0xA1,
        DAC = 0xA0, // future ???
        DIN = 0xD1, // digital input
        DOUT = 0xD0, // digital output
        DPOUT = 0xDB0 // digital pulse(beat) output
    };*/
    class HalDevice {
      protected:
        HalDevice() {}
      public:
        virtual ~HalDevice() = default;

        uint64_t uid; // this is actually 8 ascii characters coded into a 64bit value for fast lockup?
        //HAL::DeviceType type; // obsolete ???
        //uint8_t pin; // each device need to store it's own config as it can be many pins used

        virtual void read() = 0;
        virtual void write() = 0;
        virtual void loop() {};

        virtual String ToString() { return "HALDevice"; }

        
    };
}