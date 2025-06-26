
#include "HAL_JSON_OneWireTempGroup.h"

namespace HAL_JSON {

    Device* OneWireTempGroup::Create(const JsonVariant& jsonObj) {
        return new OneWireTempGroup(jsonObj);
    }

    bool OneWireTempGroup::VerifyJSON(const JsonVariant &jsonObj) {
        if (jsonObj.containsKey(HAL_JSON_KEYNAME_ITEMS) == false) {
            GlobalLogger.Error(HAL_JSON_ERR_MISSING_KEY(HAL_JSON_KEYNAME_ITEMS));
            return false;
        }
        if (jsonObj[HAL_JSON_KEYNAME_ITEMS].is<JsonArray>() == false) {
            GlobalLogger.Error(HAL_JSON_ERR_VALUE_TYPE(HAL_JSON_KEYNAME_ITEMS " not array"));
            return false;
        }
        const JsonArray items = jsonObj[HAL_JSON_KEYNAME_ITEMS].as<JsonArray>();
        if (items.size() == 0) {
            GlobalLogger.Error(HAL_JSON_ERR_ITEMS_EMPTY("OneWireTempGroup"));
            return false;
        }
        size_t itemCount = items.size();
        size_t validItemCount = 0;
        for (int i=0;i<itemCount;i++) {
            const JsonVariant item = items[i];
            if (item.is<const char*>() == false) continue; // comment item
            if (OneWireTempBus::VerifyJSON(item) == false) HAL_JSON_VALIDATE_IN_LOOP_FAIL_OPERATION;
            validItemCount++;
        }
        if (validItemCount == 0) {
            GlobalLogger.Error(HAL_JSON_ERR_ITEMS_NOT_VALID("OneWireTempGroup"));
            return false;
        }
        return true;
    }
    OneWireTempGroup::OneWireTempGroup(const JsonVariant &jsonObj) :
        autoRefresh(
            [this]() { requestTemperatures(); },
            [this]() { readAll(); })
    {
        autoRefresh.SetRefreshTimeMs(OneWireTempAutoRefresh::ParseRefreshTimeMs(jsonObj));

        // checked beforehand so extracting it here is safe
        const char* uidStr = jsonObj[HAL_JSON_KEYNAME_UID].as<const char*>();
        uid = encodeUID(uidStr);      

        // checked beforehand so extracting it here is safe
        //const char* typeStr = jsonObj[HAL_JSON_KEYNAME_TYPE].as<const char*>();
        
        // ***************** GROUP ******************
        //if (strcmp(typeStr, HAL_JSON_TYPE_ONE_WIRE_TEMP_GROUP) == 0)
        //{
            //type = OneWireTemp::Type::GROUP;
            busCount = 0;
            const JsonArray& items = jsonObj[HAL_JSON_KEYNAME_ITEMS].as<JsonArray>();
            uint32_t itemCount = items.size();
            bool* validBusses = new bool[itemCount]; // must store this as OneWireTempBus::VerifyJSON cannot be run twice as the first time it actually reserve pin use
            // first pass count valid busses
            for (int i=0;i<itemCount;i++) {
                bool valid = OneWireTempBus::VerifyJSON(items[i]);
                validBusses[i] = valid;
                if (valid == false) continue;
                busCount++;
            }
            busses = new OneWireTempBus*[busCount];
            // second pass create busses
            uint32_t index = 0;
            for (int i=0;i<itemCount;i++) {
                if (validBusses[i] == false) continue;
                busses[index++] = new OneWireTempBus(static_cast<const JsonVariant&>(items[i]));
            }
        /*}
        
        // *****************  BUS  ********************
        else if (strcmp(typeStr, HAL_JSON_TYPE_ONE_WIRE_TEMP_BUS) == 0)
        {
            type = OneWireTemp::Type::BUS;
            // there is only one bus 
            busCount = 1;
            busses = new OneWireTempBus*[1];
            busses[0] = new OneWireTempBus(jsonObj, type);
        }
        // **************** DEVICE **********************
        else if (strcmp(typeStr, HAL_JSON_TYPE_ONE_WIRE_TEMP_DEVICE) == 0)
        {
            type = OneWireTemp::Type::DEVICE;
            // allways create one default bus even if there is only one device
            // this is to avoid creating duplicate loop state machine code for each devicetype
            busCount = 1;
            busses = new OneWireTempBus*[1];
            busses[0] = new OneWireTempBus(jsonObj, type);
        }*/
    }
    OneWireTempGroup::~OneWireTempGroup() {
        if (busses != nullptr) {
            for (int i=0;i<busCount;i++)
                delete busses[i];
        }
        delete[] busses;
        busses = nullptr;
    }

    Device* OneWireTempGroup::findDevice(const UIDPath& path) { // special note: this function will not be called when the type is device as that do use uid at root level
        for (int i=0;i<busCount;i++)
        {
            Device * dev = busses[i]->findDevice(path);
            if (dev != nullptr) return dev;
        }
        return nullptr;
    }

    bool OneWireTempGroup::read(const HALReadRequest &req) {
        // TODO take into account that this may adress by bus as well
        Device* dev = findDevice(req.path);
        if (dev == nullptr) return false;
        return dev->read(req);
    }

    bool OneWireTempGroup::write(const HALWriteRequest&req) {
        // TODO take into account that this may adress by bus as well
        Device* dev = findDevice(req.path);
        if (dev == nullptr) return false;
        return dev->write(req);
    }

    void OneWireTempGroup::requestTemperatures() {
        for (int i=0;i<busCount;i++) {
            busses[i]->requestTemperatures();
        }
    }

    void OneWireTempGroup::readAll() {
        for (int i=0;i<busCount;i++) {
            busses[i]->readAll();
        }
    }

    void OneWireTempGroup::loop() {
        autoRefresh.loop();
    }
}