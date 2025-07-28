
#include "LittleFS_ext.h"

namespace LittleFS_ext {

    bool load_from_file(const char* file_name, char** outBuffer, size_t* outSize) {
        std::ifstream file(std::string(file_name), std::ios::binary | std::ios::ate);
        if (!file) return false;

        *outSize = file.tellg();
        if (*outSize == 0) return false;
        file.seekg(0);

        // Allocate mutable buffer (+1 for null terminator if needed)
        char* buffer = new char[*outSize + 1];
        file.read(buffer, *outSize);
        buffer[*outSize] = '\0'; // optional if you need null-terminated text
        *outBuffer = buffer;
        return true;
    }
}