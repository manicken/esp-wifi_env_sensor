#include "HAL_JSON_UID_Path.h"

namespace HAL_JSON {

    uint64_t encodeUID(const char* str) {
        if (!str || (strlen(str) == 0)) return 0;
        uint64_t out = 0;
        for (int i = 0; i < 8 && str[i]; ++i) {
            out |= ((uint64_t)(uint8_t)str[i]) << (8 * (7 - i)); // big-endian
        }
        return out;
    }

    uint64_t encodeUID(const char* str, uint32_t count) {
        if (!str || (strlen(str) == 0) || (count == 0)) return 0;
        uint64_t out = 0;
        for (int i = 0; i < 8 && i < count && str[i]; ++i) {
            out |= ((uint64_t)(uint8_t)str[i]) << (8 * (7 - i)); // big-endian
        }
        return out;
    }

    std::string decodeUID(uint64_t uid) {
        if (uid == 0) return "<zero>";
        if (uid == UIDPath::UID_INVALID) return "<invalid>";
        char str[9] = {}; // 8 chars + null terminator
        for (int i = 0; i < 8; ++i) {
            // Extract each byte from most significant to least significant
            str[i] = static_cast<char>((uid >> (8 * (7 - i))) & 0xFF);
            if (str[i] == '\0') { // Stop if null terminator found
                str[i] = '\0';
                break;
            }
        }
        return std::string(str);
    }

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
        items = new (std::nothrow) uint64_t[itemCount];
        if (items == nullptr) {
            delete[] indicies;
            GlobalLogger.Error(F("new UIDPath - Allocation for items failed, count: "), String(itemCount).c_str());
            itemCount = 0; // allways used at reads so setting it to zero would make reads impossible
            return;
        }
        //items2 = new HAL_UID[itemCount];
        int currStrIndex = 0;
        for (int i=0;i<itemCount;i++) {
            if (i<indiciesCount) {
                items[i] = encodeUID(&uidStr[currStrIndex], indicies[i]-currStrIndex);
                currStrIndex = indicies[i]+1;
            } else {
                items[i] = encodeUID(&uidStr[currStrIndex]);
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
    uint64_t UIDPath::getCurrentUID() {
        if (currentItemIndex == itemCount) return UID_INVALID; // ideally this wont happen
        return items[currentItemIndex];
    }
    uint64_t UIDPath::getNextUID() {
        if (itemCount == 0) return UID_INVALID; // ideally this wont happen
        if (currentItemIndex == (itemCount-1)) return UID_INVALID; // ideally this wont happen
        currentItemIndex++;
        return items[currentItemIndex];
    }
    uint64_t UIDPath::peekNextUID() {
        if (itemCount == 0) return UID_INVALID; // ideally this wont happen
        if (currentItemIndex == (itemCount-1)) return UID_INVALID; // ideally this wont happen
        return items[currentItemIndex+1];
    }
    uint64_t UIDPath::resetAndGetFirst() {
        if (itemCount == 0) return UID_INVALID; // ideally this wont happen
        currentItemIndex = 0;
        return items[0];
    }
    bool UIDPath::isLast() {
        if (itemCount == 0) return true; // ideally this wont happen
        return (currentItemIndex==(itemCount-1));
    }

    String UIDPath::ToString(ToStringType type) {
        String ret;
        for (int i=0;i<itemCount;i++) {
            if (type == ToStringType::String) {
                ret += String(decodeUID(items[i]).c_str());
            } else if (type == ToStringType::Raw) {
                ret += String(items[i],16);
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