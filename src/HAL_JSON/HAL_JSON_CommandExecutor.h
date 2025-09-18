#pragma once

#include <Arduino.h>
#include <stdlib.h>
#include "HAL_JSON_ZeroCopyString.h"
#include <queue>

#define HAL_JSON_CMD_EXEC_WRITE_CMD               "write"
#define HAL_JSON_CMD_EXEC_READ_CMD                "read"
#define HAL_JSON_CMD_EXEC_CMD                     "exec"
#define HAL_JSON_CMD_EXEC_RELOAD_CFG_JSON         "reloadcfg"
#define HAL_JSON_CMD_EXEC_RELOAD_CFG_JSON_SAFE    "reloadcfgsafe"
#define HAL_JSON_CMD_EXEC_RELOAD_SCRIPTS          "reloadscripts"
#define HAL_JSON_CMD_EXEC_PRINT_DEVICES           "printDevices"
#define HAL_JSON_CMD_EXEC_GET_AVAILABLE_GPIO_LIST "getAvailableGPIOs"
#define HAL_JSON_CMD_EXEC_PRINT_LOG_CONTENTS      "printlog"



#define HAL_JSON_CMD_EXEC_UINT32_TYPE        "uint32"
#define HAL_JSON_CMD_EXEC_BOOL_TYPE          "bool"
#define HAL_JSON_CMD_EXEC_FLOAT_TYPE         "float"
#define HAL_JSON_CMD_EXEC_JSON_STR_TYPE      "json"
#define HAL_JSON_CMD_EXEC_STRING_TYPE        "string"

//#define HAL_JSON_CommandExecutor_DEBUG_CMD

namespace HAL_JSON {
    using CommandCallback = std::function<void(const std::string& response)>;

    struct PendingRequest {
        std::string command;
        //AsyncWebServerRequest* request;
        CommandCallback cb;
    };



    class CommandExecutor {
    public:

        static std::queue<PendingRequest> g_pending;
        static portMUX_TYPE g_pendingMux;

#if defined(ESP32)
  //portMUX_TYPE g_pendingMux = portMUX_INITIALIZER_UNLOCKED;
  #define CommandExecutor_LOCK_QUEUE()   portENTER_CRITICAL(&CommandExecutor::g_pendingMux)
  #define CommandExecutor_UNLOCK_QUEUE() portEXIT_CRITICAL(&CommandExecutor::g_pendingMux)
#else
  #define CommandExecutor_LOCK_QUEUE()   noInterrupts()
  #define CommandExecutor_UNLOCK_QUEUE() interrupts()
#endif

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
        static bool reloadJSON(ZeroCopyString& zcStr, std::string& message);
        static bool writeCmd(ZeroCopyString& zcStr, std::string& message);
        static bool readCmd(ZeroCopyString& zcStr, std::string& message);
        static bool execCmd(ZeroCopyString& zcStr, std::string& message);
    };
}