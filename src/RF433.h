
#include <Arduino.h>
#include <ArduinoJson.h>
#include <stdlib.h>

namespace RF433 {

    #define RF433_FC_SHORT 470   // fixed code short
    #define RF433_FC_LONG  1410  // fixed code long
    #define RF433_FC_SYNC  14500 // fixed code sync

    #define RF433_LC_SHORT 270   // (270uS)  learning code short
    #define RF433_LC_LONG  1280  // (1.28mS) learning code data long
    #define RF433_LC_START 2650  // (2.65mS) learning code start (low part)
    #define RF433_LC_SYNC  10000 // (10mS)   learning code sync (low part)

    #define RF433_FC_REPEATS 5
    #define RF433_LC_REPEATS 5


    #define U8 uint8_t
    #define U32 uint32_t

    void SendTo433_LC_bits(U32 data, U32 mask);

    void Send433FC_One(void);
    void Send433FC_Zero(void);
    void Send433FC_Float(void);
    void Send433FC_Sync(void);

    void Send433LC_HalfOne(void);
    void Send433LC_HalfZero(void);
    void Send433LC_One(void);
    void Send433LC_Zero(void);
    void Send433LC_Sync(void);
    void Send433LC_Start(void);

    U8 Get1AsciiHex(U8 value );
    U8 Get1AsciiHexValue(U8 hex);
    U32 GetAsciiHexValue(const char *dataArrayIn, U8 nibbleCount);
    void DecodeFromJSON_SFC(const JsonVariant &json);
    void DecodeFromJSON_AFC(const JsonVariant &json);
    void init(uint8_t _pin);
    void SendTo433_SLC(const char *strUniqueId, U8 groupBtn, U8 enable, U8 btnCode);
    void SendTo433_ALC(const char *strData);
    void DecodeFromJSON_SLC(const JsonVariant &json);
    void DecodeFromJSON_ALC(const JsonVariant &json);
    void DecodeFromJSON(const JsonVariant &json);
    void DecodeFromJSON(std::string jsonStr);

}