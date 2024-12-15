
#ifndef LITTLEFS_EXT_H
#define LITTLEFS_EXT_H

#include <LittleFS.h>
namespace LittleFS_ext
{
    bool load_from_file(String file_name, String &contents);
    bool load_from_file(String file_name, char *buff);
    int getFileSize(String file_name);
    void listDir(Print &printStream, const char *dirname, uint8_t level);
    void listDir(String &str, bool isHtml, const char *dirname, uint8_t level);
}

#endif