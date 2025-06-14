#include "UIDPath.h"

namespace HAL {

    uint64_t encodeUID(const char* str) {
        uint64_t out = 0;
        for (int i = 0; i < 8 && str[i]; ++i) {
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

    UIDPath::UIDPath(const char* itemStr, const char* groupStr) {
        group = groupStr ? encodeUID(groupStr) : 0;
        item  = encodeUID(itemStr);
    }

    UIDPath::UIDPath(std::string itemStr, std::string groupStr) {
        group = groupStr.empty() ? 0 : encodeUID(groupStr.c_str());
        item  = encodeUID(itemStr.c_str());
    }

    bool UIDPath::isItemOnly() const {
        return group == 0;
    }

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
    }

}