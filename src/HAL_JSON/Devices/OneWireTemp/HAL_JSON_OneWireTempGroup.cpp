
#include "HAL_JSON_OneWireTempGroup.h"

namespace HAL_JSON {

    Device* OneWireTempGroup::Create(const JsonVariant& jsonObj, const char* type) {
        return new OneWireTempGroup(jsonObj, type);
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
        int itemCount = items.size();
        size_t validItemCount = 0;
        for (int i=0;i<itemCount;i++) {
            const JsonVariant item = items[i];
            if (IsConstChar(item) == true) continue; // comment item
            if (Device::DisabledInJson(item) == true) continue; // disabled
            if (OneWireTempBus::VerifyJSON(item) == false) HAL_JSON_VALIDATE_IN_LOOP_FAIL_OPERATION;
            validItemCount++;
        }
        if (validItemCount == 0) {
            GlobalLogger.Error(HAL_JSON_ERR_ITEMS_NOT_VALID("OneWireTempGroup"));
            return false;
        }
        return true;
    }
    OneWireTempGroup::OneWireTempGroup(const JsonVariant &jsonObj, const char* type) : Device(UIDPathMaxLength::Many, type),
        autoRefresh(
            [this]() { requestTemperatures(); },
            [this]() { readAll(); },
            ParseRefreshTimeMs(jsonObj,HAL_JSON_ONE_WIRE_TEMP_DEFAULT_REFRESHRATE_MS)
        )
    {
        const char* uidStr = GetAsConstChar(jsonObj,HAL_JSON_KEYNAME_UID);//].as<const char*>();
        uid = encodeUID(uidStr);      

        busCount = 0;
        const JsonArray& items = jsonObj[HAL_JSON_KEYNAME_ITEMS].as<JsonArray>();
        int itemCount = items.size();
        bool* validBusses = new bool[itemCount]; // must store this as OneWireTempBus::VerifyJSON cannot be run twice as the first time it actually reserve pin use
        // first pass count valid busses
        for (int i=0;i<itemCount;i++) {
            const JsonVariant& item = items[i];
            if (IsConstChar(item) == true) { validBusses[i] = false; continue; } // comment item
            if (Device::DisabledInJson(item) == true) { validBusses[i] = false; continue; } // disabled
            bool valid = OneWireTempBus::VerifyJSON(item);
            validBusses[i] = valid;
            if (valid == false) continue;
            busCount++;
        }
        busses = new OneWireTempBus*[busCount]();
        // second pass create busses
        uint32_t index = 0;
        for (int i=0;i<itemCount;i++) {
            if (validBusses[i] == false) continue;
            busses[index++] = new OneWireTempBus(static_cast<const JsonVariant&>(items[i]), type); // here type is not used so we just take the group one
        }
        delete[] validBusses;
    }
    OneWireTempGroup::~OneWireTempGroup() {
        if (busses != nullptr) {
            for (int i=0;i<busCount;i++) {
                delete busses[i];
                busses[i] = nullptr;
            }
            delete[] busses;
            busses = nullptr;
        }
    }

    Device* OneWireTempGroup::findDevice(UIDPath& path) {
        return Device::findInArray(reinterpret_cast<Device**>(busses), busCount, path, this);
        /*HAL_UID currLevelUID;

        if (uid.IsSet()) // current device uid
            currLevelUID = path.getNextUID();
        else  // current device uid == 0
            currLevelUID = path.getCurrentUID();

        if (currLevelUID.Invalid()) { GlobalLogger.Error(F("OneWireTempGroup::findDevice - currLevelUID is Invalid")); return nullptr; } // early break
        
        //HAL_JSON_DEBUG(F("OneWireTempGroup::findDevice - uid: "), decodeUID(uid).c_str());
        //HAL_JSON_DEBUG(F("OneWireTempGroup::findDevice - currLevelUID: "),decodeUID(currLevelUID).c_str());

        for (int i=0;i<busCount;i++)
        {
            OneWireTempBus* bus = busses[i];
            if (!bus) continue;  // absolute failsafe
            if (bus->uid == currLevelUID) {
                if (path.isLast()) return bus;
                //GlobalLogger.Info(F("bus->uid == currLevelUID"));
                return bus->findDevice(path); // this is the final step

            } else if (bus->uid == 0 && !path.isLast()) {
                Device* dev = bus->findDevice(path);
                if (dev != nullptr) return dev;
            }
            
        }
        return nullptr;*/
    }

    HALOperationResult OneWireTempGroup::read(const HALReadStringRequestValue& val) {
        if (val.cmd == "getAllNewDevices") { // (as json) return a list of all new devices found for all busses (this will compare against the current ones and only print new ones)
            val.out_value = "[";
            for (int i=0;i<busCount;i++) {
                OneWireTempBus* bus = busses[i];
                if (bus == nullptr) continue;
                bus->read(val);
                if (i<busCount-1)
                    val.out_value += ",";
            }
            val.out_value += "]";
            return HALOperationResult::Success;
        }
        else if (val.cmd == "getAllNewDevicesWithTemp") {
            val.out_value = "[";
            for (int i=0;i<busCount;i++) {
                OneWireTempBus* bus = busses[i];
                if (bus == nullptr) continue;
                bus->read(val);
                if (i<busCount-1)
                    val.out_value += ",";
            }
            val.out_value += "]";
            return HALOperationResult::Success;
        }
        else if (val.cmd == "getAllDevices") { // (as json) return a complete list of all devices found for all busses
            val.out_value = "[";
            for (int i=0;i<busCount;i++) {
                OneWireTempBus* bus = busses[i];
                if (bus == nullptr) continue;
                bus->read(val);
                if (i<busCount-1)
                    val.out_value += ",";
            }
            val.out_value += "]";
            return HALOperationResult::Success;
        }
        else if (val.cmd == "getAllTemperatures") { // (as json) return a complete list of all temperatures each with it's uid as the keyname and the temp as the value
            val.out_value = "[";
            for (int i=0;i<busCount;i++) {
                OneWireTempBus* bus = busses[i];
                if (bus == nullptr) continue;
                bus->read(val);
                if (i<busCount-1)
                    val.out_value += ",";
            }
            val.out_value += "]";
            return HALOperationResult::Success;
        }
        std::string stdStrCmd = val.cmd.ToString();
        GlobalLogger.Warn(F("OneWireTempGroup::read - cmd not found: "), stdStrCmd.c_str()); // this can then be read by getting the last entry from logger
        //val.out_value = F("{\"error\":\"cmd not found\"}");
        return HALOperationResult::UnsupportedCommand;  // cmd not found
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

    String OneWireTempGroup::ToString() {
        String ret;
        
        ret += DeviceConstStrings::uid;
        ret += decodeUID(uid).c_str();
        ret += "\",";
        ret += DeviceConstStrings::type;
        ret += type;
        ret += "\"";
        ret += autoRefresh.ToString();
        ret += ",\"busses\":[";
        for (int i=0;i<busCount;i++) {
            ret += "{";
            ret += busses[i]->ToString();
            ret += "}";
            if (i<busCount-1) ret += ",";
        }
        ret += "]";
        return ret;
    }
}