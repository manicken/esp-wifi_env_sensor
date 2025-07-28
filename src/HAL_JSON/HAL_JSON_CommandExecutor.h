#pragma once

#include <Arduino.h>
#include <stdlib.h>

#include "../Support/Logger.h"
#include "../Support/ConvertHelper.h"

#include "HAL_JSON_Device_GlobalDefines.h"

#include "HAL_JSON_Manager.h"

#define HAL_JSON_URL_RELOAD_JSON             (HAL_JSON_ROOT_URL "/reloadJson")
#define HAL_JSON_URL_PRINT_DEVICES           (HAL_JSON_ROOT_URL "/printDevices")

#define HAL_JSON_REST_API_WRITE_CMD          "write"
#define HAL_JSON_REST_API_READ_CMD           "read"
#define HAL_JSON_REST_API_WRITE_URL          "/" HAL_JSON_REST_API_WRITE_CMD "/"
#define HAL_JSON_REST_API_READ_URL           "/" HAL_JSON_REST_API_READ_CMD "/"

#define HAL_JSON_REST_API_UINT32_TYPE        "uint32"
#define HAL_JSON_REST_API_BOOL_TYPE          "bool"
#define HAL_JSON_REST_API_FLOAT_TYPE         "float"
#define HAL_JSON_REST_API_JSON_STR_TYPE      "json"
#define HAL_JSON_REST_API_STRING_TYPE        "string"

#define HAL_JSON_CommandExecutor_DEBUG_CMD

namespace HAL_JSON {
    class CommandExecutor {
    public:
        struct Result {
            bool success = true;
            // must be a string as the ownership must be here
            std::string message;
        };
        
        /** 
         * having ZeroCopyString as writable ref, 
         * as that would be useful when passing it to other parsing functions
         */
        static bool execute(ZeroCopyString& zcStr, std::string& message);
    private:
        struct ReadWriteCmdParameters {
            ZeroCopyString zcType;
            ZeroCopyString zcUid;
            ZeroCopyString zcValue;
            ReadWriteCmdParameters(ZeroCopyString& zcStr);
#ifdef HAL_JSON_CommandExecutor_DEBUG_CMD
            std::string ToString();
#endif
        };
        static bool writeCmd(ZeroCopyString& zcStr, std::string& message);
        static bool readCmd(ZeroCopyString& zcStr, std::string& message);
    };
}