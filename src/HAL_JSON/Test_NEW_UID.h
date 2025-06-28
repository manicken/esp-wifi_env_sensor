#include <Arduino.h>

namespace HAL_JSON {

    struct HAL_UID {
        union Test_NEW_UID
        {
            char str[8];
            uint64_t value;
        };
        
    };
}