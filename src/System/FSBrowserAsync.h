#include <Arduino.h>
#include <WiFi.h>
#include <LittleFS.h>
#include <ESPAsyncWebServer.h>

// Select the FileSystem by uncommenting one of the lines below

//#define USE_SPIFFS
//#define USE_LITTLEFS
//#define USE_SDFS
#define LITTLEFS_AND_SDMCC

#define DEBUG_UART Serial

#if defined USE_SPIFFS
#include <FS.h>
    const char* fsName = "SPIFFS";
    FS* fileSystem = &SPIFFS;
    SPIFFSConfig fileSystemConfig = SPIFFSConfig();
#elif defined USE_LITTLEFS
#include <LittleFS.h>
    const char* fsName = "LittleFS";
    FS* fileSystem = &LittleFS;
#ifdef ESP8266
    LittleFSConfig fileSystemConfig = LittleFSConfig();
#endif
#elif defined USE_SDFS
#include <SDFS.h>
    const char* fsName = "SDFS";
    FS* fileSystem = &SDFS;
    SDFSConfig fileSystemConfig = SDFSConfig();
    // fileSystemConfig.setCSPin(chipSelectPin);
#elif defined(LITTLEFS_AND_SDMCC)
#include <LittleFS.h>
#if defined(ESP32)
#include <SD_MMC.h>
#endif
    const char* fsName = "LittleFS"; // the main storage
    FS* fileSystem = nullptr;
#else
#error Please select a filesystem first by uncommenting one of the "#define USE_xxx" lines at the beginning of the sketch.
#endif



namespace FSBrowser {

    
    const char TEXT_PLAIN[] PROGMEM = "text/plain";
    const char FS_INIT_ERROR[] PROGMEM = "FS INIT ERROR";
    const char FILE_NOT_FOUND[] PROGMEM = "FileNotFound";

    char upload_html[] PROGMEM = R"=====(
<form method="post" enctype="multipart/form-data">
      <input type="file" name="name">
      <input class="button" type="submit" value="Upload">
      <br>
      <br>
      <button onclick="window.location.href='/edit'">Go to edit page</button>
      </a>
</form>

)=====";
    //const char* ssid = "YOUR_SSID";
    //const char* password = "YOUR_PASS";

    AsyncWebServer server(80);

    bool fsOK = false;

    String unsupportedFiles = String();

    /*
      Return the list of files in the directory specified by the "dir" query string parameter.
      Also demonstrates the use of chunked responses.
    */
    bool selectFileSystemAndFixPath(String &path)
    {
        //DBG_OUTPUT_PORT.println(String("before selectFileSystemAndFixPath: ") + path);
#if defined(ESP32)
        if (path.startsWith("/sdcard")) {
            fileSystem = &SD_MMC;
            path = path.substring(sizeof("/sdcard")-1);
            if (path.length() == 0) path = "/";
            //DBG_OUTPUT_PORT.println(String("sdcard new Path: ") + path);
            return true;
        }
        else 
#endif
        if (path.startsWith("/LittleFS")) {
            fileSystem = &LittleFS;
            path = path.substring(sizeof("/LittleFS")-1);
            if (path.length() == 0) path = "/";
            //DBG_OUTPUT_PORT.println(String("LittleFS new Path: ") + path);
            return true;
        }
        DEBUG_UART.println("selectFileSystemAndFixPath error - invalid filesystem" + path);
        return false;
    }
    // List files in LittleFS as JSON
    void handleList(AsyncWebServerRequest *request) {
        if (!fsOK) {
            request->send(500, "text/plain", FPSTR(FS_INIT_ERROR));
            return;
        }

        if (!request->hasParam("dir")) {
            request->send(400, "text/plain", "DIR ARG MISSING");
            return;
        }

        String path = request->getParam("dir")->value();
        if (selectFileSystemAndFixPath(path) == false) {
            request->send(200, "application/json", 
                        "[{\"type\":\"dir\",\"name\":\"sdcard\"},{\"type\":\"dir\",\"name\":\"LittleFS\"}]");
            return;
        }

        if (path != "/" && !fileSystem->exists(path)) {
            request->send(400, "text/plain", "BAD PATH");
            return;
        }

        Serial.println(String("handleFileList: ") + path);

        File dir = fileSystem->open(path, "r");
        if (!dir) {
            request->send(500, "text/plain", "Failed to open directory");
            return;
        }

        // Build JSON in RAM
        String json = "[";
        bool first = true;
        File file;

        while (file = dir.openNextFile()) {
    #ifdef USE_SPIFFS
            String error = checkForUnsupportedPath(file.name());
            if (error.length() > 0) {
                Serial.println(String("Ignoring ") + error + file.name());
                continue;
            }
    #endif
            if (!first) json += ",";
            first = false;

            json += "{\"type\":\"";
            if (file.isDirectory()) {
                json += "dir";
            } else {
                json += "file\",\"size\":\"";
                json += file.size();
            }

            json += "\",\"name\":\"";
            if (file.name()[0] == '/') {
                json += &(file.name()[1]);
            } else {
                json += file.name();
            }
            json += "\"}";
        }

        json += "]";

        request->send(200, "application/json", json);
    }


    // Handle file uploads
    void handleUpload(AsyncWebServerRequest *request, String filename, size_t index,
                    uint8_t *data, size_t len, bool final) {
    static File uploadFile;

    if (index == 0) {
        // first chunk
        if (!filename.startsWith("/")) filename = "/" + filename;
        Serial.printf("Upload start: %s\n", filename.c_str());
        uploadFile = LittleFS.open(filename, "w");
    }

    if (uploadFile) {
        uploadFile.write(data, len);
    }

    if (final) {
        if (uploadFile) uploadFile.close();
        Serial.printf("Upload complete: %s\n", filename.c_str());
    }
    }

    // Delete file
    void handleDelete(AsyncWebServerRequest *request) {
    if (!request->hasParam("file")) {
        request->send(400, "text/plain", "Missing file param");
        return;
    }
    String filename = request->getParam("file")->value();
    if (!filename.startsWith("/")) filename = "/" + filename;

    if (fileSystem->remove(filename)) {
        request->send(200, "text/plain", "Deleted " + filename);
    } else {
        request->send(500, "text/plain", "Delete failed");
    }
    }

    void handleStatus(AsyncWebServerRequest *request) {
        Serial.println("handleStatus");

        String json;
        json.reserve(256);

        json = "{\"type\":\"";
        json += fsName;
        json += "\", \"isOk\":";

        if (fsOK) {
    #ifdef ESP8266
            FSInfo fs_info;
            fileSystem->info(fs_info);
            json += F("\"true\", \"totalBytes\":\"");
            json += fs_info.totalBytes;
            json += F("\", \"usedBytes\":\"");
            json += fs_info.usedBytes;
    #elif defined(ESP32)
            json += F("\"true\", \"totalBytes\":\"");
            json += LittleFS.totalBytes();
            json += F("\", \"usedBytes\":\"");
            json += LittleFS.usedBytes();
    #endif
            json += "\"";
        } else {
            json += "\"false\"";
        }

        json += F(",\"unsupportedFiles\":\"");
        json += unsupportedFiles;
        json += "\"}";

        request->send(200, "application/json", json);
    }
    void handleFailsafeUploadPage(AsyncWebServerRequest *request) {
        request->send(200, "text/html", (const char*)upload_html);
    }


    void setup() {
 
    // Routes
    server.on("/status", HTTP_GET, handleStatus);
    server.on("/list", HTTP_GET, handleList);
    server.on("/delete", HTTP_GET, handleDelete);
    server.on("/upload", HTTP_POST, 
                [](AsyncWebServerRequest *request) {
                request->send(200, "text/plain", "Upload complete");
                },
                handleUpload);
    server.on("/edit/upload", HTTP_GET, handleFailsafeUploadPage);
    // Serve static files, index.htm as default
    server.serveStatic("/", LittleFS, "/").setDefaultFile("index.htm");
    server.onNotFound([](AsyncWebServerRequest *request) {
        if (!LittleFS.begin()) {
            request->send(500, "text/plain", "FS error");
            return;
        }
        String path = request->url();
        if (path == "/") path = "/index.htm";
        if (LittleFS.exists(path)) {
            request->send(LittleFS, path, String(), true);
        } else {
            request->send(404, "text/plain", "Not Found");
        }
    });
    server.begin();
    }

}