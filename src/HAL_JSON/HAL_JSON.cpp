
#include "HAL_JSON.h"

namespace HAL_JSON {
    Device* CreateDeviceFromJSON(JsonVariant &jsonObj) {
        const char* type = jsonObj[HAL_JSON_KEYNAME_TYPE].as<const char*>();
        for (int i=0;DeviceRegistry[i].type != nullptr;i++) {
            if (strcmp(type, DeviceRegistry[i].type) == 0) {
                if (DeviceRegistry[i].Verify_JSON_Function == nullptr) return nullptr; // silenty skip this for now (it do actually mean the list is done but not the implementation)
                if (DeviceRegistry[i].Create_Function == nullptr) return nullptr; // silenty skip this for now (it do actually mean the list is done but not the implementation) also no point of verifying the json if the create function don't exists
                auto err = DeviceRegistry[i].Verify_JSON_Function(jsonObj);
                if (err != HAL_JSON_VERIFY_JSON_RETURN_OK) {
                    Serial.println(err); // just print to the serial for now
                    return nullptr;
                }
                return DeviceRegistry[i].Create_Function(jsonObj);
            }
        }
        return nullptr; // no match
    }
}