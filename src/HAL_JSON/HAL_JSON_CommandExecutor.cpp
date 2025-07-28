
#include "HAL_JSON_CommandExecutor.h"

#ifdef _WIN32
#include <iostream>
#endif

namespace HAL_JSON {

    CommandExecutor::ReadWriteCmdParameters::ReadWriteCmdParameters(ZeroCopyString& zcStr) {
        zcType = zcStr.SplitOffHead('/');
        zcUid = zcStr.SplitOffHead('/');
        zcValue = zcStr.SplitOffHead('/');
    }
#ifdef HAL_JSON_CommandExecutor_DEBUG_CMD
    std::string CommandExecutor::ReadWriteCmdParameters::ToString() {
        std::string message;
        message += "\"Type\":\"" + zcType.ToString() + "\",";
        message += "\"UID\":\"" + zcUid.ToString() + "\",";
        message += "\"Value\":\"" + zcValue.ToString() + "\"";
        return message;
    }
#endif
    
    // TODO: refactor this function to send errors to GlobalLogger
    bool CommandExecutor::execute(ZeroCopyString& zcStr, std::string& message) {
        // Example URL: /write/uint32/tempSensor1/255 where tempSensor1 is a uid defined by cfg json
        std::cout << "zcStr:" << zcStr.ToString() << "\n";
        ZeroCopyString zcCommand = zcStr.SplitOffHead('/');

        std::cout << "zcCommand:" << zcCommand.ToString() << "\n";

#ifdef HAL_JSON_CommandExecutor_DEBUG_CMD
        message += "\"debug\":{";
        message += "\"Command\":\"" + zcCommand.ToString() + "\",";
#endif
        //bool addLastLogEntryToMessage = false;
        bool anyErrors = false;
        if (zcCommand == HAL_JSON_REST_API_WRITE_CMD)
        {
            anyErrors = writeCmd(zcStr, message) == false;
        }
        else if (zcCommand == HAL_JSON_REST_API_READ_CMD)
        {
            anyErrors = readCmd(zcStr, message) == false;
        }
        else
        {
            anyErrors = true;
            message += "\"error\":\"Unknown command.\"";
        }
        if (anyErrors) {
            const LogEntry& lastEntry = GlobalLogger.getLastEntry();
            message += "\"error\":\"";
            message += lastEntry.MessageToString().c_str();
            message += "\"";
        }
        message = "{" + message;
        message += "}";
        
        return (anyErrors == false);
    }
    //  ██     ██ ██████  ██ ████████ ███████ 
    //  ██     ██ ██   ██ ██    ██    ██      
    //  ██  █  ██ ██████  ██    ██    █████   
    //  ██ ███ ██ ██   ██ ██    ██    ██      
    //   ███ ███  ██   ██ ██    ██    ███████ 
    // 

    // TODO: refactor this function to send errors to GlobalLogger
    bool CommandExecutor::writeCmd(ZeroCopyString& zcStr, std::string& message) {
        ReadWriteCmdParameters params(zcStr);
#ifdef HAL_JSON_CommandExecutor_DEBUG_CMD
        message += params.ToString() + "},";
#endif

        if (params.zcValue.Length() == 0) {
            message += "\"error\":\"No value provided for writing.\"";
        } else {
            if (params.zcType == HAL_JSON_REST_API_BOOL_TYPE) {
                uint32_t uintValue = 0;

                if ((params.zcValue == "true") || (params.zcValue == "1")) {
                    uintValue = 1;
                } else if ((params.zcValue == "false") || (params.zcValue == "0")) {
                    uintValue = 0;
                } else {

                    message += "{\"error\":\"Invalid boolean value.\"}";
                    return false;
                }

                UIDPath uidPath(params.zcUid);
                HALValue halValue = uintValue;
                HALWriteRequest req(uidPath, halValue);
                if (Manager::write(req)) {
                    message += "\"info\":{\"Value written\":\"";
                    message += uintValue;
                    message += "\"}";
                }
                else {
                    return false;
                }
            }
            else if (params.zcType == HAL_JSON_REST_API_UINT32_TYPE) {
                // Convert value to integer
                uint32_t uintValue = 0;
                if (params.zcValue.ConvertTo_uint32(uintValue) == false) {
                    message += "{\"error\":\"Invalid uint32 value.\"}";
                } else {
                    UIDPath uidPath(params.zcUid);
                    HALValue halValue = uintValue;
                    HALWriteRequest req(uidPath, halValue);
                    if (Manager::write(req)) {
                        message += "\"info\":{\"Value written\":\"";
                        message += uintValue;
                        message += "\"}";
                    }
                    else {
                        return false;
                    }
                }

            } else if (params.zcType == HAL_JSON_REST_API_STRING_TYPE) {
                UIDPath uidPath(params.zcUid);
                std::string result;
                HALWriteStringRequestValue strHalValue(params.zcValue, result);
                
                HALWriteStringRequest req(uidPath, strHalValue);
                if (Manager::write(req)) {
                    message += "\"info\":{\"String written\":\"OK\"}";
                    //message += stdString.c_str();
                    //message += "\"}";
                }
                else {
                    return false;
                }

            } else if (params.zcType == HAL_JSON_REST_API_JSON_STR_TYPE) {
                UIDPath uidPath(params.zcUid);
                std::string result;
                HALWriteStringRequestValue strHalValue(params.zcValue, result);
                HALWriteStringRequest req(uidPath, strHalValue);
                if (Manager::write(req)) {
                    message += "\"info\":{\"Json written\":\"OK\"}";
                    //message += stdString.c_str();
                    //message += "}";
                }
                else {
                    return false;
                }
            }
            else {
                message += "\"error\":\"Unknown type for writing.\"";
                return false;
            }
        }
        return true;
    }
    //  ██████  ███████  █████  ██████  
    //  ██   ██ ██      ██   ██ ██   ██ 
    //  ██████  █████   ███████ ██   ██ 
    //  ██   ██ ██      ██   ██ ██   ██ 
    //  ██   ██ ███████ ██   ██ ██████  

    // TODO: refactor this function to send errors to GlobalLogger
    bool CommandExecutor::readCmd(ZeroCopyString& zcStr, std::string& message) {
        ReadWriteCmdParameters params(zcStr);
#ifdef HAL_JSON_CommandExecutor_DEBUG_CMD
        message += params.ToString() + "},";
#endif

        if (params.zcType == HAL_JSON_REST_API_BOOL_TYPE) {
            UIDPath uidPath(params.zcUid);
            HALValue halValue;
            HALReadRequest req(uidPath, halValue);

            if (Manager::read(req)) {
                message += DeviceConstStrings::value;
                message += halValue.asUInt();
                //message += + "\"";
            } else {
                return false;
            }
        } else if (params.zcType == HAL_JSON_REST_API_UINT32_TYPE) {
            UIDPath uidPath(params.zcUid);
            HALValue halValue;
            HALReadRequest req(uidPath, halValue);
            if (Manager::read(req)) {
                message += DeviceConstStrings::value;
                message += halValue.asUInt();
                //message += "\"";
            } else {
                return false;
            }
        } else if (params.zcType == HAL_JSON_REST_API_FLOAT_TYPE) {
            UIDPath uidPath(params.zcUid);
            if (params.zcValue.Length() == 0) {
                HALValue halValue;
                HALReadRequest req(uidPath, halValue);
            
                if (Manager::read(req)) {
                    message += DeviceConstStrings::value;
                    message += halValue.asFloat();
                    //message += "\"";
                } else {
                    return false;
                }
            }
            else {
                HALValue halValue;
                HALReadValueByCmd valByCmd(halValue, params.zcValue);
                HALReadValueByCmdReq req(uidPath, valByCmd);
            
                if (Manager::read(req)) {
                    message += DeviceConstStrings::value;
                    message += halValue.asFloat();
                    //message += "\"";
                } else {
                    return false;
                }
            }
        } else if (params.zcType == HAL_JSON_REST_API_STRING_TYPE) {
            UIDPath uidPath(params.zcUid);
            std::string result;
            HALReadStringRequestValue strHalValue(params.zcValue, result);
            
            HALReadStringRequest req(uidPath, strHalValue);
            if (Manager::read(req)) {
                message += DeviceConstStrings::value;
                message += "\"";
                message += result.c_str();
                message += "\"";
            }
            else {
                return false;
            }
        } else {
            message += "\"error\":\"Unknown type for reading.\"";
            return false;
        }
        return true;
    }
}