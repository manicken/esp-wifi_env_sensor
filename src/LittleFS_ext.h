
#ifndef LITTLEFS_EXT_H
#define LITTLEFS_EXT_H

#include <LittleFS.h>
namespace LittleFS_ext
{
    bool load_from_file(String file_name, String &contents) {
        contents = "";
        
        File this_file = LittleFS.open(file_name, "r");
        if (!this_file) { // failed to open the file, retrn empty result
            return false;
        }
        while (this_file.available()) {
            contents += (char)this_file.read();
        }
        
        this_file.close();
        return true;
    }
    bool load_from_file(String file_name, char *buff) {
        File this_file = LittleFS.open(file_name, "r");
        if (!this_file) { // failed to open the file, retrn empty result
            return false;
        }
        while (this_file.available()) {
            *buff = (char)this_file.read();
            buff++;
        }
        *buff = 0x00;
        this_file.close();
        return true;
    }
    int getFileSize(String file_name)
    {
        File this_file = LittleFS.open(file_name, "r");
        if (!this_file) { // failed to open the file, retrn empty result
            return -1;
        }
        int size = this_file.available();
        this_file.close();
        return size;
    }
}

#endif