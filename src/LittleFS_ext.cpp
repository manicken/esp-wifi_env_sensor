
#include <LittleFS.h>
#include "LittleFS_ext.h"

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

    String GetNrSpaces(int count, bool isHtml) {
        String str;
        while (count-- > 0) {
            if (isHtml) str.concat("&nbsp;");
            else str.concat(" ");
        }
        return str;
    }
    void listDir(Print &printStream, const char *dirname, uint8_t level) {
        printStream.printf(GetNrSpaces(level, false).c_str());
        printStream.printf("Listing directory: %s\r\n", dirname);
        level+=2;

        File root = LittleFS.open(dirname);
        if (!root) {
            printStream.println(" - failed to open directory");
            return;
        }
        if (!root.isDirectory()) {
            printStream.println(" - not a directory");
            return;
        }

        File file = root.openNextFile();
        while (file) {
            if (file.isDirectory()) {
                printStream.printf(GetNrSpaces(level, false).c_str());
                printStream.print("DIR : ");
                printStream.println(file.name());
                listDir(printStream, file.path(), level + 2);
                
            } else {
                printStream.printf(GetNrSpaces(level, false).c_str());
                printStream.print("FILE: ");
                printStream.print(file.name());
                printStream.print("\tSIZE: ");
                printStream.println(file.size());
            }
            file = root.openNextFile();
        }
        printStream.println();
    }
    
    void listDir(String &str, bool isHtml, const char *dirname, uint8_t level) {
        str.concat(GetNrSpaces(level, isHtml));
        str.concat("Listing directory:"); str.concat(dirname);
        if (isHtml) str.concat("<br>"); else str.concat("\r\n");
        level+=2;

        File root = LittleFS.open(dirname);
        if (!root) {
            str.concat(" - failed to open directory");
            if (isHtml) str.concat("<br>"); else str.concat("\r\n");
            return;
        }
        if (!root.isDirectory()) {
            str.concat(" - not a directory");
            if (isHtml) str.concat("<br>"); else str.concat("\r\n");
            return;
        }

        File file = root.openNextFile();
        while (file) {
            if (file.isDirectory()) {
                str.concat(GetNrSpaces(level, isHtml));
                str.concat("DIR : ");
                str.concat(file.name());
                if (isHtml) str.concat("<br>"); else str.concat("\r\n");

                listDir(str, isHtml, file.path(), level + 2);
            } else {
                str.concat(GetNrSpaces(level, isHtml));
                str.concat("FILE: ");
                str.concat(file.name());
                str.concat("\tSIZE: ");
                str.concat(file.size());
                if (isHtml) str.concat("<br>"); else str.concat("\r\n");
            }
            file = root.openNextFile();
        }
        if (isHtml) str.concat("<br>"); else str.concat("\r\n");
    }
}
