#pragma once

#include <Arduino.h>
#include <stdlib.h>

#include "../Support/Logger.h"
#include "../Support/ConvertHelper.h"

#include "HAL_JSON_Device_GlobalDefines.h"

#include "HAL_JSON_GPIO_Manager.h"
#include "HAL_JSON_Manager.h"

#define HAL_JSON_CMD_EXEC_WRITE_CMD               "write"
#define HAL_JSON_CMD_EXEC_READ_CMD                "read"
#define HAL_JSON_CMD_EXEC_RELOAD_CFG_JSON         "reloadcfg"
#define HAL_JSON_CMD_EXEC_PRINT_DEVICES           "printDevices"
#define HAL_JSON_CMD_EXEC_GET_AVAILABLE_GPIO_LIST "getAvailableGPIOs"
#define HAL_JSON_CMD_EXEC_GPIO_LIST_MODE_STRING   "string"
#define HAL_JSON_CMD_EXEC_GPIO_LIST_MODE_HEX      "hex"
#define HAL_JSON_CMD_EXEC_GPIO_LIST_MODE_BINARY   "binary"
#define HAL_JSON_CMD_EXEC_GPIO_LIST_MODE_DEFAULT  GPIO_manager::PrintListMode::Hex;

#define HAL_JSON_CMD_EXEC_UINT32_TYPE        "uint32"
#define HAL_JSON_CMD_EXEC_BOOL_TYPE          "bool"
#define HAL_JSON_CMD_EXEC_FLOAT_TYPE         "float"
#define HAL_JSON_CMD_EXEC_JSON_STR_TYPE      "json"
#define HAL_JSON_CMD_EXEC_STRING_TYPE        "string"

//#define HAL_JSON_CommandExecutor_DEBUG_CMD

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