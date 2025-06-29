
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
    OneWireTempGroup::OneWireTempGroup(const JsonVariant &jsonObj) : Device(UIDPathMaxLength::Three),
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

    Device* OneWireTempGroup::findDevice(UIDPath& path) {
        uint64_t currLevelUID = 0;
        if (uid != 0) // current device uid
            currLevelUID = path.getNextUID();
        else  // current device uid == 0
            currLevelUID = path.getCurrentUID();
        if (currLevelUID == UIDPath::UID_INVALID) return nullptr; // early break

        for (int i=0;i<busCount;i++)
        {
            OneWireTempBus* bus = busses[i];
            if (!bus) continue;  // absolute failsafe
            if (bus->uid == currLevelUID) {
                if (path.isLast()) return bus;
                return bus->findDevice(path); // this is the final step

            } else if (bus->uid == 0 && !path.isLast()) {
                Device* dev = bus->findDevice(path);
                if (dev != nullptr) return dev;
            }
            
        }
        return nullptr;
    }

    bool OneWireTempGroup::read(const HALReadStringRequestValue& val) {
        if (val.cmd == F("getAllNewDevices")) { // (as json) return a list of all new devices found for all busses (this will compare against the current ones and only print new ones)
            return false; // currently not implemented
        }
        else if (val.cmd == F("getAllDevices")) { // (as json) return a complete list of all devices found for all busses
            val.out_value = "{";
            for (int i=0;i<busCount;i++) {
                OneWireTempBus* bus = busses[i];
                if (bus == nullptr) continue;
                bus->read(val);
                if (i<busCount-1)
                    val.out_value += ",";
            }
            val.out_value += "}";
            return true;
        }
        else if (val.cmd == F("getAllTemperatures")) { // (as json) return a complete list of all temperatures each with it's uid as the keyname and the temp as the value
            return false; // currently not implemented
        }
        val.out_value = F("{\"error\":\"cmd not found\"}");
        return true;  // cmd not found
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