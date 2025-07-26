#include "HAL_JSON_UID_Path.h"

namespace HAL_JSON {
    

    UIDPath::UIDPath() = default;

    UIDPath::~UIDPath() {
        if (items != nullptr)
            delete[] items;
    }

    UIDPath::UIDPath(const char* uidStr) {
        currentItemIndex = 0;
        if (uidStr == nullptr || strlen(uidStr) == 0) {
            itemCount = 0; // allways used at reads so setting it to zero would make reads impossible
            GlobalLogger.Error(F("new UIDPath - input uidStr invalid"));
            return;
        }
        uint32_t indiciesCount = 0;
        const uint32_t* indicies = CharArray::getIndicies(uidStr, ':', indiciesCount);
        itemCount = indiciesCount + 1;
        items = new (std::nothrow) HAL_UID[itemCount];
        if (items == nullptr) {
            delete[] indicies;
            GlobalLogger.Error(F("new UIDPath - Allocation for items failed, count: "), std::to_string(itemCount).c_str());
            itemCount = 0; // allways used at reads so setting it to zero would make reads impossible
            return;
        }
        //items2 = new HAL_UID[itemCount];
        int currStrIndex = 0;
        for (uint32_t i=0;i<itemCount;i++) {
            if (i<indiciesCount) {
                items[i] = encodeUID(&uidStr[currStrIndex], indicies[i]-currStrIndex);
                currStrIndex = indicies[i]+1;
            } else {
                items[i] = encodeUID(&uidStr[currStrIndex]);
            }
        }
        
        delete[] indicies;
    }
    UIDPath::UIDPath(const CharArray::ZeroCopyString& uidzcStr) {
        currentItemIndex = 0;
        if (uidzcStr.Length() == 0) {
            itemCount = 0; // allways used at reads so setting it to zero would make reads impossible
            GlobalLogger.Error(F("new UIDPath - input uidStr invalid"));
            return;
        }
        uint32_t indiciesCount = 0;
        const uint32_t* indicies = CharArray::getIndicies(uidzcStr, ':', indiciesCount);
        
        itemCount = indiciesCount + 1;
        items = new (std::nothrow) HAL_UID[itemCount];
        if (items == nullptr) {
            delete[] indicies;
            GlobalLogger.Error(F("new UIDPath - Allocation for items failed, count: "), std::to_string(itemCount).c_str());
            itemCount = 0; // allways used at reads so setting it to zero would make reads impossible
            return;
        }
        //items2 = new HAL_UID[itemCount];
        int currStrIndex = 0;
        for (uint32_t i=0;i<itemCount;i++) {
            if (i<indiciesCount) {
                items[i] = encodeUID(&uidzcStr.start[currStrIndex], indicies[i]-currStrIndex);
                currStrIndex = indicies[i]+1;
            } else {
                items[i] = encodeUID(&uidzcStr.start[currStrIndex], &uidzcStr.end[currStrIndex]-&uidzcStr.start[currStrIndex]);
            }
        }
        
        delete[] indicies;
    }

    UIDPath::UIDPath(const std::string& uidStr) : UIDPath(uidStr.c_str()) { }
    
    bool UIDPath::empty() const {
        return (!items || itemCount == 0);
    }
    
    uint32_t UIDPath::count() {
        return itemCount;
    }
    HAL_UID UIDPath::getCurrentUID() {
        if (currentItemIndex == itemCount) return HAL_UID::UID_INVALID; // ideally this wont happen
        return items[currentItemIndex];
    }
    HAL_UID UIDPath::getNextUID() {
        if (itemCount == 0) return HAL_UID::UID_INVALID; // ideally this wont happen
        if (currentItemIndex == (itemCount-1)) return HAL_UID::UID_INVALID; // ideally this wont happen
        currentItemIndex++;
        return items[currentItemIndex];
    }
    HAL_UID UIDPath::peekNextUID() {
        if (itemCount == 0) return HAL_UID::UID_INVALID; // ideally this wont happen
        if (currentItemIndex == (itemCount-1)) return HAL_UID::UID_INVALID; // ideally this wont happen
        return items[currentItemIndex+1];
    }
    HAL_UID UIDPath::resetAndGetFirst() {
        if (itemCount == 0) return HAL_UID::UID_INVALID; // ideally this wont happen
        currentItemIndex = 0;
        return items[0];
    }
    bool UIDPath::isLast() {
        if (itemCount == 0) return true; // ideally this wont happen
        return (currentItemIndex==(itemCount-1));
    }

    std::string UIDPath::ToString(ToStringType type) {
        std::string ret;
        for (uint32_t i=0;i<itemCount;i++) {
            if (type == ToStringType::String) {
                ret += std::string(decodeUID(items[i]).c_str());
            } else if (type == ToStringType::Raw) {
                ret += Convert::toHex(items[i].val);
            }
            if (i<itemCount-1)
                ret += ":";
        }
        return ret;
    }
   /* uint64_t UIDPath::byIndex(uint32_t index) {
        return items[index];
    }*/
}