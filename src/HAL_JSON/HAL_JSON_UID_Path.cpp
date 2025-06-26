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

    UIDPath::UIDPath(const char* uidStr) {
        uint32_t indiciesCount = 0;
        const uint32_t* indicies = CharArray::getIndicies(uidStr, ':', indiciesCount);
        itemCount = indiciesCount + 1;
        items = new uint64_t[itemCount];
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

    /*UIDPath::UIDPath(const char* uidStr) {
        const char* sep = strchr(uidStr, ':');
        if (sep) {
            std::string groupPart(uidStr, sep - uidStr);
            std::string itemPart(sep + 1);
            group = encodeUID(groupPart.c_str());
            item = encodeUID(itemPart.c_str());
        } else {
            group = 0;
            item = encodeUID(uidStr);
        }
    }*/

    UIDPath::UIDPath(const std::string& uidStr) : UIDPath(uidStr.c_str()) { }

    uint64_t UIDPath::root() const {
        if (!items || itemCount == 0) return UID_INVALID;
        return items[0];
    }
    uint64_t UIDPath::last() const {
        if (!items || itemCount == 0) return UID_INVALID;
        return items[itemCount-1];
    }
    uint64_t UIDPath::byIndex(uint32_t index) const {
        if (!items || itemCount == 0 || index >= itemCount) return UID_INVALID;
        return items[index];
    }
/* don't think i need theese operations
    bool UIDPath::operator==(const UIDPath& other) const {
        return group == other.group && item == other.item;
    }

    bool UIDPath::operator==(const char* uidStr) const {
        const char* sep = strchr(uidStr, ':');
        if (sep) {
            std::string groupPart(uidStr, sep - uidStr);
            std::string itemPart(sep + 1);
            return group == encodeUID(groupPart.c_str()) &&
                   item == encodeUID(itemPart.c_str());
        } else {
            return group == 0 && item == encodeUID(uidStr);
        }
    }

    bool UIDPath::operator==(const std::string& uidStr) const {
        return *this == uidStr.c_str();
    }*/

}