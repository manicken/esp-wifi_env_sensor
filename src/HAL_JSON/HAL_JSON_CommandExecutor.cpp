
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
        //std::cout << "zcStr:" << zcStr.ToString() << "\n";
        ZeroCopyString zcCommand = zcStr.SplitOffHead('/');

        //std::cout << "zcCommand:" << zcCommand.ToString() << "\n";

#ifdef HAL_JSON_CommandExecutor_DEBUG_CMD
        message += "\"debug\":{";
        message += "\"Command\":\"" + zcCommand.ToString() + "\",";
#endif
        //bool addLastLogEntryToMessage = false;
        bool anyErrors = false;
        if (zcCommand == HAL_JSON_CMD_EXEC_WRITE_CMD)
        {
            anyErrors = writeCmd(zcStr, message) == false;
        }
        else if (zcCommand == HAL_JSON_CMD_EXEC_READ_CMD)
        {
            anyErrors = readCmd(zcStr, message) == false;
        }
        else if (zcCommand == HAL_JSON_CMD_EXEC_RELOAD_CFG_JSON) {
            ZeroCopyString zcOptionalFileName = zcStr.SplitOffHead('/');
#ifdef HAL_JSON_CommandExecutor_DEBUG_CMD
            message += "\"filename\":\"" + (zcOptionalFileName.Length() != 0?zcOptionalFileName.ToString():"default") + "\"}";
#endif
            std::string filePath;
            if (zcOptionalFileName.Length() == 0) {
                filePath = "hal/cfg.json";
            } else {
                filePath = "hal/" + zcOptionalFileName.ToString();
            }
#ifdef _WIN32
            std::cout << "Reload cfg json: " << filePath << std::endl;  
#endif
        
            if (Manager::ReadJSON(filePath.c_str())) {
                message += "\"info\":\"OK\"";
            } else {
                message += "\"info\":\"FAIL\",";
                anyErrors = true;
            }
            
        }
        else if (zcCommand == HAL_JSON_CMD_EXEC_GET_AVAILABLE_GPIO_LIST) {
            message += GPIO_manager::GetList(zcStr);
        }
        else if (zcCommand == HAL_JSON_CMD_EXEC_PRINT_DEVICES) {
            message += Manager::ToString();
        }
        else if (zcCommand == HAL_JSON_CMD_EXEC_PRINT_LOG_CONTENTS) {
            GlobalLogger.printAllLogs(Serial);
        }
        else
        {
            anyErrors = true;
            message += "\"error\":\"Unknown command.\"";
            message += ",\"command\":\""+zcCommand.ToString()+"\"";
        }
        if (anyErrors) {

            try {
                const LogEntry& lastEntry = GlobalLogger.getLastEntry();
                String lastEntryStr = lastEntry.MessageToString();
                message += "\"error\":\"";
                message += lastEntryStr.c_str();
                message += "\"";
                
            } catch (const std::exception& e) {
                message += "\"info\":\"EXCEPTION: ";
                message += e.what();
                message += "\",";
                anyErrors = true;
            }
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
            return false;
        } else {
            HALOperationResult  writeResult = HALOperationResult::UnsupportedOperation;

            if (params.zcType == HAL_JSON_CMD_EXEC_BOOL_TYPE) {
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

                writeResult = Manager::write(req);
                if (writeResult == HALOperationResult::Success) {
                    message += "\"info\":{\"Value written\":\"";
                    message += std::to_string(uintValue);
                    message += "\"}";
                }
            }
            else if (params.zcType == HAL_JSON_CMD_EXEC_UINT32_TYPE) {
                // Convert value to integer
                uint32_t uintValue = 0;
                if (params.zcValue.ConvertTo_uint32(uintValue) == false) {
                    message += "{\"error\":\"Invalid uint32 value.\"}";
                } else {
                    UIDPath uidPath(params.zcUid);
                    HALValue halValue = uintValue;
                    HALWriteRequest req(uidPath, halValue);

                    writeResult = Manager::write(req);
                    if (writeResult == HALOperationResult::Success) {
                        message += "\"info\":{\"Value written\":\"";
                        message += std::to_string(uintValue);
                        message += "\"}";
                    }
                }

            } else if (params.zcType == HAL_JSON_CMD_EXEC_FLOAT_TYPE) {
                // Convert value to integer
                float floatValue = 0.0f;
                if (params.zcValue.ConvertTo_float(floatValue) == false) {
                    message += "{\"error\":\"Invalid float value.\"}";
                } else {
#ifdef _WIN32
                    std::cout << "float value written: " << floatValue << "\n";
#endif
                    UIDPath uidPath(params.zcUid);
                    HALValue halValue = floatValue;
                    HALWriteRequest req(uidPath, halValue);

                    writeResult = Manager::write(req);
                    if (writeResult == HALOperationResult::Success) {
                        message += "\"info\":{\"Value written\":\"";
                        message += std::to_string(floatValue);
                        message += "\"}";
                    }
                }

            } else if (params.zcType == HAL_JSON_CMD_EXEC_STRING_TYPE) {
                UIDPath uidPath(params.zcUid);
                std::string result;
                HALWriteStringRequestValue strHalValue(params.zcValue, result);
                
                HALWriteStringRequest req(uidPath, strHalValue);

                writeResult = Manager::write(req);
                if (writeResult == HALOperationResult::Success) {
                    message += "\"info\":{\"String written\":\"OK\"}";
                    //message += stdString.c_str();
                    //message += "\"}";
                }

            } else if (params.zcType == HAL_JSON_CMD_EXEC_JSON_STR_TYPE) {
                UIDPath uidPath(params.zcUid);
                std::string result;
                HALWriteStringRequestValue strHalValue(params.zcValue, result);
                HALWriteStringRequest req(uidPath, strHalValue);

                writeResult = Manager::write(req);
                if (writeResult == HALOperationResult::Success) {
                    message += "\"info\":{\"Json written\":\"OK\"}";
                    //message += stdString.c_str();
                    //message += "}";
                }
            }
            else {
                message += "\"error\":\"Unknown type for writing.\"";
                return false;
            }
            if (writeResult != HALOperationResult::Success) {
                message += "\"error\":\"";
                message += ToString(writeResult);
                message += "\"";
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
        std::string valueStr;
#ifdef HAL_JSON_CommandExecutor_DEBUG_CMD
        message += params.ToString() + "},";
#endif
        HALOperationResult  readResult = HALOperationResult::UnsupportedOperation;

        if (params.zcType == HAL_JSON_CMD_EXEC_BOOL_TYPE) {
            UIDPath uidPath(params.zcUid);
            HALValue halValue;
            HALReadRequest req(uidPath, halValue);

            readResult = Manager::read(req);
            if (readResult == HALOperationResult::Success) {
                valueStr = std::to_string(halValue.asUInt());
            }
        } else if (params.zcType == HAL_JSON_CMD_EXEC_UINT32_TYPE) {
            UIDPath uidPath(params.zcUid);
            HALValue halValue;
            HALReadRequest req(uidPath, halValue);

            readResult = Manager::read(req);
            if (readResult == HALOperationResult::Success) {
                valueStr = std::to_string(halValue.asUInt());
            }
        } else if (params.zcType == HAL_JSON_CMD_EXEC_FLOAT_TYPE) {
            UIDPath uidPath(params.zcUid);
            if (params.zcValue.Length() == 0) {
                HALValue halValue;
                HALReadRequest req(uidPath, halValue);

                readResult = Manager::read(req);
                if (readResult == HALOperationResult::Success) {
                    valueStr = std::to_string(halValue.asFloat());
                }
            }
            else {
                HALValue halValue;
                HALReadValueByCmd valByCmd(halValue, params.zcValue);
                HALReadValueByCmdReq req(uidPath, valByCmd);

                readResult = Manager::read(req);
                if (readResult == HALOperationResult::Success) {
                    valueStr = std::to_string(halValue.asFloat());
                }
            }
        } else if (params.zcType == HAL_JSON_CMD_EXEC_STRING_TYPE) {
            UIDPath uidPath(params.zcUid);
            std::string result;
            HALReadStringRequestValue strHalValue(params.zcValue, result);
            HALReadStringRequest req(uidPath, strHalValue);

            readResult = Manager::read(req);
            if (readResult == HALOperationResult::Success) {
                valueStr = "\"";
                valueStr += result;
                valueStr += "\"";
            }
        } else {
            message += "\"error\":\"Unknown type for reading.\"";
            return false;
        }
        if (readResult != HALOperationResult::Success) {
            message += "\"error\":\"";
            message += ToString(readResult);
            message += "\"";
            return false;
        }
        message += DeviceConstStrings::value;
        message += valueStr;
        return true;
    }
}