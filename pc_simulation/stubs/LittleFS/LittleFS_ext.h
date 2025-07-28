
#pragma once

//#include <Arduino.h>
#include <fstream>
namespace LittleFS_ext {

    bool load_from_file(const char* file_name, char** outBuffer, size_t* outSize);

}