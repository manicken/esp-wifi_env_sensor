
#include <Arduino.h>

#if defined(ESP32)
//#define CONST_STRINGS_USE_F_PREFIX
#endif

#ifdef CONST_STRINGS_USE_F_PREFIX
#define CONST_STR_DECLARE(name) extern const __FlashStringHelper* name
#define CONST_STR_DEFINE(name, value) const __FlashStringHelper* name = F(value)
#else
#define CONST_STR_DECLARE(name) extern const char* name
#define CONST_STR_DEFINE(name, value) const char* name = value
#endif

namespace CONSTSTR {
    CONST_STR_DECLARE(htmlContentType_TextPlain);
}