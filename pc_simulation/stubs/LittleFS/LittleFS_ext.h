
#pragma once

//#include <Arduino.h>
#include <fstream>
namespace LittleFS_ext {

    enum class FileResult {
        Success,
        FileNotFound,
        FileEmpty,
        AllocFail,
        FileReadError
    };
    FileResult load_from_file(const char* file_name, char** outBuffer, size_t* outSize);

}