
#ifndef LITTLEFS_EXT_H
#define LITTLEFS_EXT_H

#include <LittleFS.h>
namespace LittleFS_ext
{
    enum class FileResult {
        Success,
        FileNotFound,
        FileEmpty,
        AllocFail,
        FileReadError
    };

    FileResult load_from_file(const char* file_name, String &contents);
    /** please note that using this function is not safe,
     * and buff size must be bigger or equal to filesize + 1
     * as otherwise it will overwrite data outside buff
     * and this will likely corrupt data to lead to undefined
     * behaviour
     */
    FileResult load_from_file(const char* file_name, char *buff);
    /** using this is safe */
    FileResult load_from_file(const char* file_name, char** outBuffer, size_t* outSize = nullptr);
    int getFileSize(const char* file_name);
    void listDir(Stream &printStream, const char *dirname, uint8_t level);
    void listDir(String &str, bool isHtml, const char *dirname, uint8_t level);
}

#endif