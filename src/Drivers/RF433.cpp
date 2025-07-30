
#include "RF433.h"

namespace RF433
{
    int pin = -1;
    uint32_t RF433_FC_SHORT = 470;   // fixed code short
    uint32_t RF433_FC_LONG = 1410;   // fixed code long
    uint32_t RF433_FC_SYNC = 14500;  // fixed code sync
    uint32_t RF433_LC_SHORT = 270;   // (270uS)  learning code short
    uint32_t RF433_LC_LONG = 1280;   // (1.28mS) learning code data long
    uint32_t RF433_LC_START = 2650;  // (2.65mS) learning code start (low part)
    uint32_t RF433_LC_REPEAT_DELAY = 10000;  // (10mS)   learning code sync (low part)
    uint32_t RF433_FC_REPEATS = 5;
    uint32_t RF433_LC_REPEATS = 5;

    
    #define RF433_Set() digitalWrite(pin, HIGH)
    #define RF433_Clear() digitalWrite(pin, LOW)

    void init(arch_word_t _pin)
    {
        pin = _pin;
        pinMode(pin, OUTPUT); // output
    }
    arch_word_t Get1AsciiHex(arch_word_t value ) // converts only the lower nibble
    {
        value = value & 0x0F;
        if (value <= 0x09)
            return (value + 0x30);
        else
            return (value + 0x37);
    }

    arch_word_t Get1AsciiHexValue(arch_word_t hex)
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

    arch_word_t GetAsciiHexValue(const char *dataArrayIn, arch_word_t nibbleCount)
    {
        uint32_t value = 0;
        uint32_t nibblePower = 1;
        uint32_t nibbleValue = 0;
        arch_word_t i = 0;
        
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

    uint32_t decode5AlphaNumericTo4byteId(const std::string& id) {
        if (id.length() != 4) return 0; // Handle error or return special value

        auto charToValue = [](char c) -> int {
            if (c >= '0' && c <= '9') return c - '0';                // 0–9 → 0–9
            if (c >= 'a' && c <= 'z') return c - 'a' + 10;           // a–z → 10–35
            if (c >= 'A' && c <= 'Z') return c - 'A' + 36;           // A–Z → 36–61
            if (c == ' ') return 62;                                 // space → 62
            return -1; // Invalid
        };

        uint32_t result = 0;
        for (char c : id) {
            int val = charToValue(c);
            if (val == -1) return 0; // or handle invalid character
            result = (result << 6) | val;
        }

        return result;
    }

    //  ███████ ██ ██   ██ ███████ ██████       ██████  ██████  ██████  ███████ 
    //  ██      ██  ██ ██  ██      ██   ██     ██      ██    ██ ██   ██ ██      
    //  █████   ██   ███   █████   ██   ██     ██      ██    ██ ██   ██ █████   
    //  ██      ██  ██ ██  ██      ██   ██     ██      ██    ██ ██   ██ ██      
    //  ██      ██ ██   ██ ███████ ██████       ██████  ██████  ██████  ███████ 

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
    
    void SendTo433_FC_bits(uint32_t data, uint32_t mask) // start bit can be 0 - 31 and the data is sent from MSB startbit to LSB
    {
        if (pin == -1) return;
        do
        {
            //LED7_IO = 1;
            if ((data & mask) == mask)
                Send433FC_Zero(); // fixed code do send zero as ones
            else
                Send433FC_Float(); // fixed code do send ones as floating
            mask /= 2; // shift mask from left to right
            //LED7_IO = 0;
            
        } while (mask);
    }

    void SendTo433_FC(uint32_t data) {
        if (pin == -1) return;
        for (uint32_t i = 0; i < RF433_FC_REPEATS; i++)
        {
            // sync
            Send433FC_Sync();
            SendTo433_FC_bits(data, (1u << 11));

        }
    }

    void SendTo433_FC(uint32_t staticData, arch_word_t state)
    {
        if (state == 0)
            staticData |= 0x01;
        else
            staticData &= 0xFFE;
        SendTo433_FC(staticData);
    }

    //  ██      ███████  █████  ██████  ███    ██ ██ ███    ██  ██████       ██████  ██████  ██████  ███████ 
    //  ██      ██      ██   ██ ██   ██ ████   ██ ██ ████   ██ ██           ██      ██    ██ ██   ██ ██      
    //  ██      █████   ███████ ██████  ██ ██  ██ ██ ██ ██  ██ ██   ███     ██      ██    ██ ██   ██ █████   
    //  ██      ██      ██   ██ ██   ██ ██  ██ ██ ██ ██  ██ ██ ██    ██     ██      ██    ██ ██   ██ ██      
    //  ███████ ███████ ██   ██ ██   ██ ██   ████ ██ ██   ████  ██████       ██████  ██████  ██████  ███████

    void Send433LC_One(void)
    {
        RF433_Set();
        delayMicroseconds(RF433_LC_SHORT);
        RF433_Clear();
        delayMicroseconds(RF433_LC_LONG);
        RF433_Set();
        delayMicroseconds(RF433_LC_SHORT);
        RF433_Clear();
        delayMicroseconds(RF433_LC_SHORT);
    }

    void Send433LC_Zero(void)
    {
        RF433_Set();
        delayMicroseconds(RF433_LC_SHORT);
        RF433_Clear();
        delayMicroseconds(RF433_LC_SHORT);
        RF433_Set();
        delayMicroseconds(RF433_LC_SHORT);
        RF433_Clear();
        delayMicroseconds(RF433_LC_LONG);
    }

    void Send433LC_Start(void)
    {
        RF433_Set();
        delayMicroseconds(RF433_LC_SHORT);
        RF433_Clear();
        delayMicroseconds(RF433_LC_START);
    }

    void SendTo433_LC_bits(uint32_t data, uint32_t mask) // start bit can be 0 - 31 and the data is sent from MSB startbit to LSB
    {
        if (pin == -1) return;
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

    void SendTo433_LC(uint32_t data) // 433MHz "Advanced Learning Code" (NEXA)
    {
        if (pin == -1) return;
        uint32_t i;
        
        for (i = 0; i < RF433_LC_REPEATS; i++)
        {
            Send433LC_Start();
            SendTo433_LC_bits(data, VALUE_2_POW_31);
            RF433_Set();
            delayMicroseconds(RF433_LC_SHORT);
            RF433_Clear();
            if (i<RF433_LC_REPEATS-1) // don't delay after the last is sent
                delayMicroseconds(RF433_LC_REPEAT_DELAY);
        }
    }

    void SendTo433_LC(uint32_t staticData, arch_word_t state)
    {
        if (state == 1)
            staticData |= 0x10;
        else
            staticData &= 0xFFFFFFEF;
        SendTo433_LC(staticData);
    }

    //  ███████ ███████ ███    ██ ██████      ██████  ██    ██          ██ ███████  ██████  ███    ██ 
    //  ██      ██      ████   ██ ██   ██     ██   ██  ██  ██           ██ ██      ██    ██ ████   ██ 
    //  ███████ █████   ██ ██  ██ ██   ██     ██████    ████            ██ ███████ ██    ██ ██ ██  ██ 
    //       ██ ██      ██  ██ ██ ██   ██     ██   ██    ██        ██   ██      ██ ██    ██ ██  ██ ██ 
    //  ███████ ███████ ██   ████ ██████      ██████     ██         █████  ███████  ██████  ██   ████ 

    bool Validate_FC_JSON(const JsonVariant &jsonObj) {
        if (!HAL_JSON::ValidateJsonStringField(jsonObj,"ch")) return false;
        if (!HAL_JSON::ValidateJsonStringField(jsonObj,"btn")) return false;
        if (!HAL_JSON::ValidateJsonStringField(jsonObj,"state")) return false;
        return true;
    }

    uint32_t Get433_SFC_Data(const JsonVariant &jsonObj) {
        const char* chStr = HAL_JSON::GetAsConstChar(jsonObj,"ch");
        const char* btnStr = HAL_JSON::GetAsConstChar(jsonObj,"btn");
        const char* stateStr = HAL_JSON::GetAsConstChar(jsonObj,"state");

        arch_word_t ch = chStr[0];
        arch_word_t button = btnStr[0];

        uint32_t data = 0;
        if (ch & 0x01) data |= (1u << 11);
        else if (ch & 0x02) data |= (1u << 10);
        else if (ch & 0x04) data |= (1u << 9);
        else if (ch & 0x08) data |= (1u << 8);
        if (button & 0x01) data |= (1u << 7);
        else if (button & 0x02) data |= (1u << 6);
        else if (button & 0x04) data |= (1u << 5);
        else if (button & 0x08) data |= (1u << 4);
        if (stateStr[0] == '1') data |= (1u << 0);
        return data;
    }
    uint32_t Get433_AFC_Data(const JsonVariant &jsonObj) {
        const char* chStr = HAL_JSON::GetAsConstChar(jsonObj,"ch");
        const char* btnStr = HAL_JSON::GetAsConstChar(jsonObj,"btn");
        const char* stateStr = HAL_JSON::GetAsConstChar(jsonObj,"state");

        arch_word_t button = Get1AsciiHexValue(btnStr[0]);
        arch_word_t ch = Get1AsciiHexValue(chStr[0]);

        uint32_t data = 0;
        if (ch & 0x01) data |= (1u << 11);
        if (ch & 0x02) data |= (1u << 10);
        if (ch & 0x04) data |= (1u << 9);
        if (ch & 0x08) data |= (1u << 8);
        if (button & 0x01) data |= (1u << 7);
        if (button & 0x02) data |= (1u << 6);
        if (button & 0x04) data |= (1u << 5);
        if (button & 0x08) data |= (1u << 4);
        if (stateStr[0] == '0') data |= (1u << 0);
        return data;
    }
    
    void DecodeFromJSON_SFC(const JsonVariant &jsonObj)
    {
        if (pin == -1) return;
        if (Validate_FC_JSON(jsonObj) == false) return;
        SendTo433_FC(Get433_SFC_Data(jsonObj));
    }
    void DecodeFromJSON_AFC(const JsonVariant &jsonObj)
    {
        if (pin == -1) return;
        if (Validate_FC_JSON(jsonObj) == false) return;
        SendTo433_FC(Get433_AFC_Data(jsonObj));
    }
    void GetLCSettings(const JsonVariant &jsonObj)
    {
        RF433_LC_REPEAT_DELAY = HAL_JSON::GetAsUINT32(jsonObj, "pl_sync", RF433_LC_REPEAT_DELAY);//.as<uint32_t>();
        RF433_LC_START = HAL_JSON::GetAsUINT32(jsonObj, "pl_start", RF433_LC_START);//.as<uint32_t>();
        RF433_LC_SHORT = HAL_JSON::GetAsUINT32(jsonObj, "pl_short", RF433_LC_SHORT);//.as<uint32_t>();
        RF433_LC_LONG = HAL_JSON::GetAsUINT32(jsonObj, "pl_long", RF433_LC_LONG);//.as<uint32_t>();
        RF433_LC_REPEATS = HAL_JSON::GetAsUINT32(jsonObj, "pl_repeats", RF433_LC_REPEATS);//.as<uint32_t>();
    }
    uint32_t Get433_LC_Data(const JsonVariant &jsonObj) {
        uint32_t data = 0;
        if (jsonObj.containsKey(HAL_JSON_KEYNAME_TX433_HEXID)){
            data = RF433::GetAsciiHexValue(HAL_JSON::GetAsConstChar(jsonObj, "hexid"), 6);
        } else if (jsonObj.containsKey(HAL_JSON_KEYNAME_TX433_ALPHA_NUMERIC_ID)) {
            data = RF433::decode5AlphaNumericTo4byteId(HAL_JSON::GetAsConstChar(jsonObj, "anid"));
        } else {
            data = 0; // this will never happen if VerifyJSON is used beforehand
        }
        data &= 0xFFFFFF00; // mask out msb of unit id
        data |= 0x80; // constant bit of unit id
        // 0x40 constant bit of unit id
        if (jsonObj.containsKey(HAL_JSON_KEYNAME_TX433_GRP_BTN) && HAL_JSON::IsUINT32(jsonObj,HAL_JSON_KEYNAME_TX433_GRP_BTN)) {
            uint32_t grp_btn = HAL_JSON::GetAsUINT32(jsonObj,HAL_JSON_KEYNAME_TX433_GRP_BTN);
            if (grp_btn == 1)
                data |= 0x20;
        }
        if (jsonObj.containsKey(HAL_JSON_KEYNAME_TX433_STATE) && HAL_JSON::IsUINT32(jsonObj,HAL_JSON_KEYNAME_TX433_STATE)) {
            uint32_t state = HAL_JSON::GetAsUINT32(jsonObj,HAL_JSON_KEYNAME_TX433_STATE);
            if (state == 1)
                data |= 0x10;
        }
        if (jsonObj.containsKey(HAL_JSON_KEYNAME_TX433_BTN) && HAL_JSON::IsUINT32(jsonObj,HAL_JSON_KEYNAME_TX433_BTN)) {
            uint32_t btn = HAL_JSON::GetAsUINT32(jsonObj,HAL_JSON_KEYNAME_TX433_BTN);
            btn &= 0x0F;
            data |= btn;
        }
        return data;
    }
    void DecodeFromJSON_LC(const JsonVariant &jsonObj)
    {
        if (pin == -1) return;
        GetLCSettings(jsonObj);
        if (VerifyLC_JSON(jsonObj) == false) return;
        uint32_t data = Get433_LC_Data(jsonObj);
        if (data == 0) return; // invalid data
        SendTo433_LC(data);
    }

    void DecodeFromJSON(const JsonVariant &jsonObj) {
        if (pin == -1) return;
        if (!jsonObj.containsKey("type")) return;
        const char* type = HAL_JSON::GetAsConstChar(jsonObj,"type");
            
        if (CharArray::equalsIgnoreCase(type,"sfc")) //simple fixed code
            DecodeFromJSON_SFC(jsonObj);
        else if (CharArray::equalsIgnoreCase(type,"afc")) // advanced fixed code (this is only usable with hardware hacked devices or when adress pins are individually selectable)
            DecodeFromJSON_AFC(jsonObj);
        else if (CharArray::equalsIgnoreCase(type,"lc")) // learning code
            DecodeFromJSON_LC(jsonObj);
    }

    /*void DecodeFromJSON(const String& jsonStr)
    {
        if (pin == -1) return;
        size_t jsonDocBufferSize = (size_t)((float)jsonStr.length() * 1.5f);
        DynamicJsonDocument jsonObj(jsonDocBufferSize);
        deserializeJson(jsonObj, jsonStr.c_str());
        DecodeFromJSON(jsonObj);
    }*/
    void DecodeFromJSON(std::string jsonStr)
    {
        if (pin == -1) return;
        size_t jsonDocBufferSize = (size_t)((float)jsonStr.length() * 1.5f);
        DynamicJsonDocument jsonObj(jsonDocBufferSize);
        deserializeJson(jsonObj, jsonStr.c_str());
        DecodeFromJSON(jsonObj);
    }
    bool VerifyLC_JSON(const JsonVariant &jsonObj) {
        if (jsonObj.containsKey(HAL_JSON_KEYNAME_TX433_HEXID)) {
            if (HAL_JSON::ValidateJsonStringField_noContains(jsonObj, HAL_JSON_KEYNAME_TX433_HEXID) == false) return false;
        } else if (jsonObj.containsKey(HAL_JSON_KEYNAME_TX433_ALPHA_NUMERIC_ID)) {
            if (HAL_JSON::ValidateJsonStringField_noContains(jsonObj, HAL_JSON_KEYNAME_TX433_ALPHA_NUMERIC_ID) == false) return false;
        } else {
            GlobalLogger.Error(F("TX433unit - LC - no unit id defined"));
            return false;
        } 
        // grp_btn, btn and state are optional fields
        return true;
    }
}