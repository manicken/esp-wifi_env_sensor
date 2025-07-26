#include "CharArrayHelpers.h"

namespace CharArray {
    ZeroCopyString::ZeroCopyString() {
        start = nullptr;
        end = nullptr;
    }
    size_t ZeroCopyString::Length() const {
        if (start == nullptr || end == nullptr || end < start ) return 0;
        return end-start;
    }
    std::string ZeroCopyString::ToString() const {
        return std::string(start, end);
    }

    uint32_t countChar(const char* str, char ch) {
        uint32_t count = 0;
        while (*str) {
            if (*str == ch) count++;
            str++;
        }
        return count;
    }
    uint32_t countChar(const ZeroCopyString& zcStr, char ch) {
        if (zcStr.Length() == 0) return 0;
        uint32_t count = 0;
        const char* p = zcStr.start;
        const char* const end = zcStr.end;
        while (p < end) {
            if (*p == ch) count++;
            p++;
        }
        return count;
    }
    const uint32_t* getIndicies(const ZeroCopyString& zcStr, char ch, uint32_t& outCount) {
        if (zcStr.Length() == 0) return nullptr;
        outCount = countChar(zcStr, ch);
        if (outCount == 0) return nullptr;
        uint32_t* indicies = new uint32_t[outCount];
        uint32_t index = 0;
        uint32_t strIndex = 0;
        const char* p = zcStr.start;
        const char* const end = zcStr.end;
        while (p < end) {
            if (*p == ch) { 
                indicies[index++] = strIndex;
            }
            p++;
            strIndex++;
        }
        return indicies;
    }
    const uint32_t* getIndicies(const char* str, char ch, uint32_t& outCount) {
        outCount = countChar(str, ch);
        if (outCount == 0) return nullptr;
        uint32_t* indicies = new uint32_t[outCount];
        uint32_t index = 0;
        uint32_t strIndex = 0;
        while (*str) {
            if (*str == ch) { 
                indicies[index++] = strIndex;
            }
            str++;
            strIndex++;
        }
        return indicies;
    }

    const char** getPointers(const char* str, char ch, uint32_t& outCount) {
        outCount = countChar(str, ch);
        if (outCount == 0) return nullptr;
        const char** indicies = new const char*[outCount];
        uint32_t index = 0;
        while (*str) {
            if (*str == ch) { 
                indicies[index++] = str;
            }
            str++;
            //strIndex++;
        }
        return indicies;
    }

    

    bool equalsIgnoreCase(const char* a, const char* b) {
        while (*a && *b) {
            if (std::tolower(*a) != std::tolower(*b)) {
                return false;
            }
            ++a;
            ++b;
        }
        return *a == *b; // Ensure both strings ended
    }

    // Counts tokens in the input string without modifying the original
    int count_tokens(const char* str, const char* delimiters) {
        int count = 0;
        bool inToken = false;
        while (*str) {
            if (strchr(delimiters, *str)) {
                inToken = false;
            } else if (!inToken) {
                inToken = true;
                ++count;
            }
            ++str;
        }
        return count;
    }

    // Simplified pseudo-implementation
    //static char *current_position = NULL;

    char *my_strtok(char **str, const char *delimiters) {
        if (*str == NULL) {
            return NULL;
        }

        // Skip leading delimiters
        *str += strspn(*str, delimiters);
        if (**str == '\0') {
            return NULL;
        }

        char *token_start = *str;

        // Find end of token
        *str += strcspn(*str, delimiters);

        if (**str != '\0') {
            **str = '\0';   // Null-terminate token
            (*str)++;       // Advance past null terminator
        } else {
            *str = NULL;    // End of string reached
        }

        return token_start;
    }

    bool EndsWith(const char* str, const char* suffix) {
        if (!str || !suffix) return false;

        size_t str_len = strlen(str);
        size_t suffix_len = strlen(suffix);

        if (suffix_len > str_len) return false;

        return strcmp(str + (str_len - suffix_len), suffix) == 0;
    }

/*
    StringView::StringView() : ptr(nullptr), len(0) {}
    StringView::StringView(const char* p, size_t l) : ptr(p), len(l) {}
    bool StringView::empty() const { return len == 0; }
    std::string StringView::toString() const { return std::string(ptr, len); } // for debug/logging
    */
}