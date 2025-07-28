
#include "LittleFS.h"

LittleFS_class::LittleFS_class() {}

bool LittleFS_class::exists(const char* path) {
    std::ifstream file(path);
    return file.good();    
}

LittleFS_class LittleFS;