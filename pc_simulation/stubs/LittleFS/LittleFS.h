#pragma once

#include <Arduino.h>
#include <fstream>

class LittleFS_class {
public:
    LittleFS_class();
    bool exists(const char* path);
};

extern LittleFS_class LittleFS;