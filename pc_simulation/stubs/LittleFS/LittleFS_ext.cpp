
#include "LittleFS_ext.h"
#include <iostream>

namespace LittleFS_ext {

    bool load_from_file(const char* file_name, char** outBuffer, size_t* outSize) {
        
        std::ifstream file(std::string(file_name), std::ios::binary | std::ios::ate);
        if (!file) {
            std::cout << "file not found:" << file_name << "\n";
            return false;
        } 

        *outSize = file.tellg();
        if (*outSize == 0) {
            std::cout << "file is empty:" << file_name << "\n";
            return false;
        }
        file.seekg(0);

        // Allocate mutable buffer (+1 for null terminator if needed)
        char* buffer = new char[*outSize + 1];
        file.read(buffer, *outSize);
        buffer[*outSize] = '\0'; // optional if you need null-terminated text
        *outBuffer = buffer;
        return true;
    }
}