/* 
 
*/
#include "main.h"

#include "System/WifiManagerWrapper.h"
#include "System/System.h"
#include "Support/ConstantStrings.h"
#include "Drivers/HearbeatLed.h" // this should not be here in final version (should only be accessible through HAL interface)

AsyncWebServer webserver(HTTP_PORT);

#ifdef ESP8266
#define LITTLEFS_BEGIN_FUNC_CALL LittleFS.begin()
#elif ESP32
#define AUTOFORMAT_ON_FAIL true
#define LITTLEFS_BEGIN_FUNC_CALL LittleFS.begin(AUTOFORMAT_ON_FAIL, "/LittleFS", 10, "spiffs")
#endif

unsigned long currTime = 0;

#if defined(USE_DISPLAY)

#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(128, 64, &Wire, -1); // -1 = no reset pin
void init_display(void);
#endif

void Timer_SyncTime() {
    DEBUG_UART.println("Timer_SyncTime");
    NTP::NTPConnect();
    tmElements_t now2;
    breakTime(time(nullptr), now2);
    int year = (int)now2.Year + 1970;
    setTime(now2.Hour+1, now2.Minute, now2.Second, now2.Day, now2.Month, year);
}

#if defined(HAL_JSON_H_)
void Alarm_SendToDeviceManager(const OnTickExtParameters *param)
{
    DEBUG_UART.println("Alarm_SendToDeviceManager");
    const AsStringParameter* casted_param = static_cast<const AsStringParameter*>(param);
    if (casted_param != nullptr)
    {
       // DeviceManager::DecodeFromJSON(casted_param->jsonStr);
    }
}
#endif

Scheduler::NameToFunction nameToFunctionList[] = {
//   name         , onTick            , onTickExt
    {"ntp_sync"   , &Timer_SyncTime   , nullptr           }
#if defined(HAL_JSON_H_)
    ,{"devmgr"     , nullptr           , &Alarm_SendToDeviceManager}
#endif
};

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

void setup() {
    if (Info::resetReason_is_crash(false)) {
        OTA::setup();
        failsafeLoop();
    }

    DEBUG_UART.printf("free @ start:%u\n",ESP.getFreeHeap());
    DEBUG_UART.begin(115200);
    DEBUG_UART.setDebugOutput(true);
    DEBUG_UART.println(F("\r\n!!!!!Start of MAIN Setup!!!!!\r\n"));
    DEBUG_UART.println(Info::getResetReasonStr());

    

    if (LITTLEFS_BEGIN_FUNC_CALL == true) FSBrowser::fsOK = true; // this call is needed before all access to internal Flash file system

   // MainConfig::begin(webserver);

#if defined(ESP32) && defined(FSBROWSER_SYNCED_WS_H_)
    if (InitSD_MMC()) FSBrowser::fsOK = true;
#endif
#if defined(USE_DISPLAY)
    init_display();
#endif
    WiFi.setSleep(false);
    //connect_to_wifi();
#ifdef WIFI_MANAGER_WRAPPER_H_
    WiFiManager_Handler(display);
#endif
    OTA::setup();

    Scheduler::setup(webserver, nameToFunctionList, sizeof(nameToFunctionList) / sizeof(nameToFunctionList[0]));

    Info::startTime = now();

    initWebServerHandlers();
#ifdef FSBROWSER_SYNCED_WS_H_
    FSBrowser::setup(webserver);
#else
    FSBrowser::setup(webserver);
#endif

   // ThingSpeak::setup(webserver);
    Info::setup(webserver);
    

    HeartbeatLed::setup(webserver);
#if defined(ESP32)
    Start_MDNS();
#endif
    /* TODO move this into a hal json device
    
    /*
#if defined(ESP32)
    File test = SD_MMC.open("/StartTimes.log", "a", true);
    test.println(Time_ext::GetTimeAsString(now()).c_str());
    test.close();
#endif
*/
#ifdef HAL_JSON_H_
    HAL_JSON::begin();
#endif
    webserver.begin();
    // make sure that the following are allways at the end of this function
    DEBUG_UART.printf("free end of setup:%u\n",ESP.getFreeHeap());
    DEBUG_UART.println(F("\r\n!!!!!End of MAIN Setup!!!!!\r\n"));
}

void loop() {
#ifdef HAL_JSON_H_
    HAL_JSON::loop();
#endif
    ArduinoOTA.handle();
    Scheduler::HandleAlarms();
    HeartbeatLed::task();

#if defined(ESP8266)
    MDNS.update(); // this is only required on esp8266
#endif
    
    if (WiFi.status() != wl_status_t:: WL_CONNECTED)
    {
        DEBUG_UART.println("WiFi connection lost.");
        wl_status_t status = WiFi.begin();
        if (status == wl_status_t::WL_CONNECTED) DEBUG_UART.println("reconnect OK");
        else DEBUG_UART.println("reconnect fail");
        
        //connect_to_wifi();
    }
}

void initWebServerHandlers(void)
{
    webserver.on(MAIN_URLS_FORMAT_LITTLE_FS, [](AsyncWebServerRequest* req) {
        
        if (LittleFS.format()) {
            if (!LITTLEFS_BEGIN_FUNC_CALL) {
                req->send(500, "text/html", "Format OK, but mount failed");
                return;
            }
            req->send(200, "text/html", "Format OK and mounted");
        } else {
            req->send(500, "text/html", "Format failed");
        }
    });
    webserver.on(MAIN_URLS_MKDIR, [](AsyncWebServerRequest* req) {
        if (!req->hasArg("dir")) { req->send(200,"text/html", "Error: dir argument missing"); }
        else
        {
            String path = req->arg("dir");
            
            if (LittleFS.mkdir(path.c_str()) == false) {
                req->send(200,"text/html", "Error: could not create dir:" + path);
            }
            else
            {
                req->send(200,"text/html", "create new dir OK");
            }

        }

    });
#if defined(ESP32)
    webserver.on("/sdcard_listfiles", [](AsyncWebServerRequest* req) {
        
        //if (SD_MMC.begin("/sdcard", true, false, 20000)) {
            File root = SD_MMC.open("/");
            if (!root) {
                req->send(200, CONSTSTR::htmlContentType_TextPlain, "error while open sd card again");
                return;
            }

            File file;
            String ret;
            while (file = root.openNextFile())
            {
                ret.concat("Name:"); ret.concat(file.name());
                ret.concat(", Size:"); ret.concat(file.size());
                ret.concat(", Dir:"); ret.concat(file.isDirectory()?"true":"false");
                ret.concat("\n");
            }
            req->send(200, CONSTSTR::htmlContentType_TextPlain, ret.c_str());
       // }else {webserver.send(200, CONSTSTR::htmlContentType_TextPlain, "could not open sd card a second time");}
    });
#endif
    webserver.on("/crashTest", [](AsyncWebServerRequest* req) {
        req->send(200, CONSTSTR::htmlContentType_TextPlain, "The system will now crash!!!, and luckily go into failsafe OTA upload mode.");
        int *ptr = nullptr; // Null pointer
        *ptr = 42;          // Dereference the null pointer (causes a crash)
    });

    /*
    webserver.onNotFound([](AsyncWebServerRequest* req) {                              // If the client requests any URI
        String uri = req->uri();
        DEBUG_UART.println("onNotFound - hostHeader:" + req->hostHeader());
        //bool isDir = false;
        //DEBUG_UART.println("webserver on not found:" + uri);
        if (uri.startsWith("/LittleFS") == false && uri.startsWith("/sdcard") == false)
        {
#if defined(ESP32)
            File fileToCheck = LittleFS.open(uri);
#elif defined(ESP8266)
            File fileToCheck = LittleFS.open(uri, "r");
#endif
             if (!fileToCheck) {
                webserver.send(404, CONSTSTR::htmlContentType_TextPlain, F("404: Not Found")); // otherwise, respond with a 404 (Not Found) error
                return;
            }

            if (fileToCheck.isDirectory()) // if it's a folder, try to find index.htm or index.html
            {
                if (LittleFS.exists(uri + "/index.html"))
                    uri += "/index.html";
                else if (LittleFS.exists(uri + "/index.htm"))
                    uri += "/index.htm";
            }
            fileToCheck.close();
            uri = "/LittleFS" + uri; // default to LittleFS
        }
#ifdef FSBROWSER_SYNCED_WS_H_
        if (!FSBrowser::handleFileRead(uri))                  // send it if it exists
            webserver.send(404, CONSTSTR::htmlContentType_TextPlain, F("404: Not Found")); // otherwise, respond with a 404 (Not Found) error
#endif
    });
    */
}
#if defined(USE_DISPLAY)
void init_display(void)
{
    delay(1000);
    Wire.begin(21, 22, 100000); // SDA=21, SCL=22
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        DEBUG_UART.println(F("OLED init fail"));
        return;
    }

    DEBUG_UART.println(F("OLED OK"));
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,0);
    //display.println(F("Hello ESP32!"));
    display.display(); // <--- push buffer to screen
}
#endif
