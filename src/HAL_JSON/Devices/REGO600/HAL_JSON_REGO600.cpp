
#include "HAL_JSON_REGO600.h"

namespace HAL_JSON {
    
    REGO600::REGO600(const JsonVariant &jsonObj, const char* type) : Device(UIDPathMaxLength::One,type) {

        size_t itemCount = 5; // this is just a demo and need to be set from actual count
        requestList = new (std::nothrow) Drivers::REGO600::Request*[itemCount];
        registerItems = new (std::nothrow) REGO600register*[itemCount];
        for (int i=0;i<itemCount;i++) {
            registerItems[i] = new REGO600register(jsonObj["items"][i], nullptr);
            // here value is passed by ref so that REGO600 driver can access and change the value so REGO600register read function can then get the correct value
            requestList[i] = new Drivers::REGO600::Request(registerItems[i]->opcode, registerItems[i]->address, registerItems[i]->value); 
        }
        rego600 = new Drivers::REGO600(rxPin, txPin, requestList, itemCount);
    }
    REGO600::~REGO600() {
        if (rego600 != nullptr)
            delete rego600;
        if (requestList != nullptr) { // if for example the allocation did fail
            for (auto i=0;i<registerItemCount; i++) {
                delete requestList[i];
            }
            delete[] requestList;
        }
        if (registerItems != nullptr) { // if for example the allocation did fail
            for (auto i=0;i<registerItemCount; i++) {
                delete registerItems[i];
            }
            delete[] registerItems;
        }
        pinMode(rxPin, INPUT);
        pinMode(txPin, INPUT);
    }
    void REGO600::begin() {
        rego600->begin(); // this will initialize a first request
    }
    void REGO600::loop() {
        rego600->loop();
    }

    bool REGO600::VerifyJSON(const JsonVariant &jsonObj) {

        // following cannot be used in this context as now we need to use two pins
        //return GPIO_manager::ValidateJsonAndCheckIfPinAvailableAndReserve(jsonObj, (static_cast<uint8_t>(GPIO_manager::PinMode::OUT) | static_cast<uint8_t>(GPIO_manager::PinMode::IN)));
        return false;
    }

    Device* REGO600::Create(const JsonVariant &jsonObj, const char* type) {
        return new REGO600(jsonObj, type);
    }

    String REGO600::ToString() {
        String ret;
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += type;
        ret += "\"";
        return ret;
    }

}