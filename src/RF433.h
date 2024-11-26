
#include <Arduino.h>
#include <ArduinoJson.h>
#include <stdlib.h>

namespace RF433 {
    uint8_t pin = 14;

    #define RF433_FC_SHORT 470   // fixed code short
    #define RF433_FC_LONG  1410  // fixed code long
    #define RF433_FC_SYNC  14500 // fixed code sync

    #define RF433_LC_SHORT 270   // (270uS)  learning code short
    #define RF433_LC_LONG  1280  // (1.28mS) learning code data long
    #define RF433_LC_START 2650  // (2.65mS) learning code start (low part)
    #define RF433_LC_SYNC  10000 // (10mS)   learning code sync (low part)

    #define RF433_FC_REPEATS 5
    #define RF433_LC_REPEATS 5

    #define RF433_Set() digitalWrite(pin, HIGH)
    #define RF433_Clear() digitalWrite(pin, LOW)
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

    void init(uint8_t _pin)
    {
        pin = _pin;
        pinMode(pin, OUTPUT);
    }
    U8 Get1AsciiHex(U8 value ) // converts only the lower nibble
    {
        value = value & 0x0F;
        if (value <= 0x09)
            return (value + 0x30);
        else
            return (value + 0x37);
    }

    U8 Get1AsciiHexValue(U8 hex)
    {
        if (hex >= '0' && hex <= '9')
            return hex - 0x30;
        else if (hex >= 'A' && hex <= 'F')
            return hex - 0x37;
        else if (hex >= 'a' && hex <= 'f')
            return hex - 0x57;
        else
            return 0x00;
    }

    U32 GetAsciiHexValue(const char *dataArrayIn, U8 nibbleCount)
    {
        U32 value = 0;
        U32 nibblePower = 1;
        U32 nibbleValue = 0;
        U8 i = 0;
        
        dataArrayIn += (nibbleCount - 1);
        //LED7_IO = 1;
        while (i < nibbleCount)
        {
            nibbleValue = Get1AsciiHexValue(*dataArrayIn);
            value += (nibbleValue * nibblePower);
            nibblePower *= 16;
            dataArrayIn--;
            i++;
        }
        //LED7_IO = 0;
        return value;
    }

    void Send433FC_One(void)
    {
        RF433_Set();
        delayMicroseconds(RF433_FC_LONG);
        RF433_Clear();
        delayMicroseconds(RF433_FC_SHORT);
        RF433_Set();
        delayMicroseconds(RF433_FC_LONG);
        RF433_Clear();
        delayMicroseconds(RF433_FC_SHORT);
    }

    void Send433FC_Zero(void)
    {
        RF433_Set();
        delayMicroseconds(RF433_FC_SHORT);
        RF433_Clear();
        delayMicroseconds(RF433_FC_LONG);
        RF433_Set();
        delayMicroseconds(RF433_FC_SHORT);
        RF433_Clear();
        delayMicroseconds(RF433_FC_LONG);
    }

    void Send433FC_Float(void)
    {
        RF433_Set();
        delayMicroseconds(RF433_FC_SHORT);
        RF433_Clear();
        delayMicroseconds(RF433_FC_LONG);
        RF433_Set();
        delayMicroseconds(RF433_FC_LONG);
        RF433_Clear();
        delayMicroseconds(RF433_FC_SHORT);
    }

    void Send433FC_Sync(void)
    {
        RF433_Set();
        delayMicroseconds(RF433_FC_SHORT);
        RF433_Clear();
        delayMicroseconds(14500);
    }

    void Send433LC_HalfOne(void)
    {
        RF433_Set();
        delayMicroseconds(RF433_LC_SHORT);
        RF433_Clear();
        delayMicroseconds(RF433_LC_SHORT);
    }

    void Send433LC_HalfZero(void)
    {
        RF433_Set();
        delayMicroseconds(RF433_LC_SHORT);
        RF433_Clear();
        delayMicroseconds(RF433_LC_LONG);
    }

    void Send433LC_One(void)
    {
        Send433LC_HalfZero();
        Send433LC_HalfOne();
    }

    void Send433LC_Zero(void)
    {
        Send433LC_HalfOne();
        Send433LC_HalfZero();
    }

    void Send433LC_Sync(void)
    {
        RF433_Set();
        delayMicroseconds(RF433_LC_SHORT);
        RF433_Clear();
        delayMicroseconds(RF433_LC_SYNC);
    }

    void Send433LC_Start(void)
    {
        RF433_Set();
        delayMicroseconds(RF433_LC_SHORT);
        RF433_Clear();
        delayMicroseconds(RF433_LC_START);
    }

    void SendTo433_AFC(char ch_, char button_, char on_off) // 433MHz Advanced Fixed Code "NEXA"
    {
        //AVR32_GPIO.port[0].gpers = UART2_TXD_PIN_MASK; // GPIO uses UART_TXD pin (PA26)
        int i;
        U8 button = Get1AsciiHexValue(button_);
        U8 ch = Get1AsciiHexValue(ch_);

        for (i = 0; i < RF433_FC_REPEATS; i++)
        {
            // sync
            Send433FC_Sync();
            // channel
            if (ch & 0x01) Send433FC_Zero();
            else           Send433FC_Float();
            if (ch & 0x02) Send433FC_Zero();
            else           Send433FC_Float();
            if (ch & 0x04) Send433FC_Zero();
            else           Send433FC_Float();
            if (ch & 0x08) Send433FC_Zero();
            else           Send433FC_Float();
            // button
            if (button & 0x01) Send433FC_Zero();
            else               Send433FC_Float();
            if (button & 0x02) Send433FC_Zero();
            else               Send433FC_Float();
            if (button & 0x04) Send433FC_Zero();
            else               Send433FC_Float();
            if (button & 0x08) Send433FC_Zero();
            else               Send433FC_Float();
            // on/off
            Send433FC_Float();
            Send433FC_Float();
            Send433FC_Float();
            if (on_off == '1') Send433FC_Float();
            else               Send433FC_Zero();
        }

    }
    void SendTo433_SFC(char ch, char button, char on_off) // 433MHz Simple Fixed Code "NEXA"
    {
    // AVR32_GPIO.port[0].gpers = UART2_TXD_PIN_MASK; // GPIO uses UART_TXD pin (PA26)
        int i;

        for (i = 0; i < RF433_FC_REPEATS; i++)
        {
            //LED6_IO = 1;
            Send433FC_Sync();
            // channel
            if (ch == '1') Send433FC_Zero();
            else           Send433FC_Float();
            if (ch == '2') Send433FC_Zero();
            else           Send433FC_Float();
            if (ch == '3') Send433FC_Zero();
            else           Send433FC_Float();
            if (ch == '4') Send433FC_Zero();
            else           Send433FC_Float();
            // button
            if (button == '1') Send433FC_Zero();
            else               Send433FC_Float();
            if (button == '2') Send433FC_Zero();
            else               Send433FC_Float();
            if (button == '3') Send433FC_Zero();
            else               Send433FC_Float();
            if (button == '4') Send433FC_Zero();
            else               Send433FC_Float();
            // on/off
            Send433FC_Float();
            Send433FC_Float();
            Send433FC_Float();
            if (on_off == '1') Send433FC_Float();
            else               Send433FC_Zero();
            // sync
            //LED6_IO = 0;
        }

    }

    void SendTo433_LC_bits(U32 data, U32 mask) // start bit can be 0 - 31 and the data is sent from MSB startbit to LSB
    {
        do
        {
            //LED7_IO = 1;
            if ((data & mask) == mask)
                Send433LC_One();
            else
                Send433LC_Zero();
            mask /= 2; // shift mask from left to right
            //LED7_IO = 0;
            
        } while (mask);
    }

    /* example json
    {
    "cmd": "RF433",
    "type":"slc",
    "uid":"3AD4FF6",
    "btn":"F",
    "state":"0",
    "grp_btn":"0"
    }
    */

    void SendTo433_SLC(const char *strUniqueId, U8 groupBtn, U8 enable, U8 btnCode) // 433MHz "Simple Learning Code" (NEXA)
    {
        int i;
        U32 uId = GetAsciiHexValue(strUniqueId, 7);
        btnCode = Get1AsciiHexValue(btnCode);
        
        for (i = 0; i < RF433_LC_REPEATS; i++)
        {
            //LED6_IO = 1;
            Send433LC_Start();
            //SendTo433_LC_bits(0x2AAAAAAA, 0x02000000); // this works
            SendTo433_LC_bits(uId, 0x02000000); // works not
            if (groupBtn == '1')
                Send433LC_One();
            else
                Send433LC_Zero();
            if (enable == '1')
                Send433LC_One();
            else
                Send433LC_Zero();
            SendTo433_LC_bits(btnCode, 0x08);
            Send433LC_Sync();
            //LED6_IO = 0;
        }
    }

    void SendTo433_ALC(const char *strData) // 433MHz "Advanced Learning Code" (NEXA)
    {
        int i;
        U32 data = GetAsciiHexValue(strData, 8);
        for (i = 0; i < RF433_LC_REPEATS; i++)
        {
            Send433LC_Start();
            SendTo433_LC_bits(data, 0x80000000);
            Send433LC_Sync();
        }
    }

    
    void DecodeFromJSON_SFC(const JsonVariant &json)
    {
        if (!json.containsKey("ch")) return;
        std::string ch = json["ch"];
        if (!json.containsKey("btn")) return;
        std::string btn = json["btn"];
        if (!json.containsKey("state")) return;
        std::string state = json["state"];
        SendTo433_SFC(ch[0], btn[0], state[0]);
    }
    void DecodeFromJSON_AFC(const JsonVariant &json)
    {
        if (!json.containsKey("ch")) return;
        std::string ch = json["ch"];
        if (!json.containsKey("btn")) return;
        std::string btn = json["btn"];
        if (!json.containsKey("state")) return;
        std::string state = json["state"];
        SendTo433_AFC(ch[0], btn[0], state[0]);
    }
    void DecodeFromJSON_SLC(const JsonVariant &json)
    {
        std::string grp_btn = "", btn = "";
        Serial1.println("slc type");
        if (!json.containsKey("uid")) return;
        std::string uid = json["uid"];
        if (json.containsKey("grp_btn")) // grp_btn can be '1' or '0'
            
            grp_btn = (std::string)json["grp_btn"].as<std::string>();
        else
            grp_btn = "0";
        if (!json.containsKey("state")) return;
        std::string state = json["state"];
        if (json.containsKey("btn")) // can be any number 1-4
            btn = (std::string)json["btn"].as<std::string>();
        else
            btn = "0";
        Serial1.println("slc sending");
        SendTo433_SLC(uid.c_str(), grp_btn[0], state[0], btn[0]);
    }
    void DecodeFromJSON_ALC(const JsonVariant &json)
    {
        if (!json.containsKey("raw")) return;
        std::string raw = json["raw"];
        SendTo433_ALC(raw.c_str());
    }

    void DecodeFromJSON(const JsonVariant &json) {

        if (!json.containsKey("type")) return;
        std::string type = json["type"];

        for (char &c : type)
            c = std::tolower(c);
            
        if (type == "sfc") //simple fixed code
            DecodeFromJSON_SFC(json);
        else if (type == "afc") // advanced fixed code
            DecodeFromJSON_AFC(json);
        else if (type == "slc") // simple learning code
            DecodeFromJSON_SLC(json);
        else if (type == "alc") // advanced learning code
            DecodeFromJSON_ALC(json);
    }

    void DecodeFromJSON(std::string jsonStr)
    {
        StaticJsonDocument<128> json;
        deserializeJson(json, jsonStr.c_str());
        DecodeFromJSON(json);
    }
}