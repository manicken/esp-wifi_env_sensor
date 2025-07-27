
#include "HAL_JSON_GPIO_Manager_REST.h"

namespace HAL_JSON {
    namespace GPIO_manager
    {
        namespace REST {
            WEBSERVER_TYPE *webserver = nullptr;

            void setup(WEBSERVER_TYPE &srv) {
                webserver = &srv;
                srv.on(HAL_JSON_GPIO_MANAGER_GET_AVAILABLE_GPIO_LIST, HTTP_GET, sendList);
            }

            void sendList()
            {
                String srv_return_msg = "{";
                PrintListMode listMode = PrintListMode::Hex;
                if (webserver->hasArg("mode")) {
                    String listModeStr = webserver->arg("mode");
                    if (listModeStr == "string")
                        listMode = PrintListMode::String;
                    else if (listModeStr == "binary")
                        listMode = PrintListMode::Binary;
                    else
                        listMode = PrintListMode::Hex; // default
                }
        #if defined(ESP8266)
                srv_return_msg.concat("\"MCU\":\"ESP8266\",");
        #elif defined(ESP32)
                srv_return_msg.concat("\"MCU\":\"ESP32\",");
        #endif
                if (listMode != PrintListMode::String) {
                    srv_return_msg.concat("\"PinModes\":{");
                    if (PinModeStrings_length == -1) set_PinModeStrings_length();
                    for (int i=0;i<PinModeStrings_length;i++)
                    {
                        srv_return_msg.concat("\"");
                        uint8_t modeMask = PinModeStrings[i].mode;
                        if (listMode == PrintListMode::Binary) {
                            String binStr = String(modeMask, BIN);
                            while (binStr.length() < 8) binStr = "0" + binStr;  // pad to 8 bits
                            srv_return_msg.concat(binStr);
                        }
                        else {
                            String hexStr = String(modeMask, HEX);
                            while (hexStr.length() < 2) hexStr = "0" + hexStr;  // pad to 2 hex digits
                            srv_return_msg.concat(hexStr);
                        }
                        srv_return_msg.concat("\":\"");
                        srv_return_msg.concat(PinModeStrings[i].Name);
                        srv_return_msg.concat("\"");
                        if (i<(PinModeStrings_length-1))
                            srv_return_msg.concat(",");
                    }
                    srv_return_msg.concat("},");
                }
                srv_return_msg.concat("\"list\":{");
                bool first = true;
                if (available_gpio_list_lenght == -1) set_available_gpio_list_length();
                for (int i=0;i<available_gpio_list_lenght;i++)
                {
                    if (first == false)
                        srv_return_msg.concat(",");
                    else
                        first = false;

                    srv_return_msg.concat("\"");
                    srv_return_msg.concat(available_gpio_list[i].pin);
                    srv_return_msg.concat("\":\"");
                    uint8_t modeMask = available_gpio_list[i].mode;
                    if (listMode == PrintListMode::String)
                        srv_return_msg.concat(describePinMode(modeMask).c_str());
                    else if (listMode == PrintListMode::Binary) {
                        String binStr = String(modeMask, BIN);
                        while (binStr.length() < 8) binStr = "0" + binStr;  // pad to 8 bits
                        srv_return_msg.concat(binStr);
                    }
                    else { // (listMode == PrintListMode::Hex) 
                        String hexStr = String(modeMask, HEX);
                        while (hexStr.length() < 2) hexStr = "0" + hexStr;  // pad to 2 hex digits
                        srv_return_msg.concat(hexStr);
                    }
                    srv_return_msg.concat("\"");                        
                }
                srv_return_msg.concat("}");
                srv_return_msg.concat("}");
                webserver->send(200, "text/json", srv_return_msg);
            }
        }
    }
}