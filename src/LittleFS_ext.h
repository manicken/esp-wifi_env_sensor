
#ifndef LITTLEFS_EXT_H
#define LITTLEFS_EXT_H

#include <LittleFS.h>
namespace LittleFS_ext
{
    bool load_from_file(const String& file_name, String &contents);
    /** please note that using this function is not safe,
     * and buff size must be bigger or equal to filesize + 1
     * as otherwise it will overwrite data outside buff
     * and this will likely corrupt data to lead to undefined
     * behaviour
     */
    bool load_from_file(const String& file_name, char *buff);
    /** using this is safe */
    bool load_from_file(const String& file_name, char** outBuffer, size_t* outSize = nullptr);
    int getFileSize(const String& file_name);
    void listDir(Print &printStream, const char *dirname, uint8_t level);
    void listDir(String &str, bool isHtml, const char *dirname, uint8_t level);
}

#endif