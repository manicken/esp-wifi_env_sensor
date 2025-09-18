/* 
 
*/
#include "main.h"

#include "HAL_JSON/HAL_JSON_CommandExecutor.h"

#include "System/WifiManagerWrapper.h"
#include "System/System.h"
#include "Support/ConstantStrings.h"
#include "Drivers/HearbeatLed.h" // this should not be here in final version (should only be accessible through HAL interface)

#include "esp_task_wdt.h"

AsyncWebServer webserver(HTTP_PORT);

#ifdef ESP8266
#define LITTLEFS_BEGIN_FUNC_CALL LittleFS.begin()
#elif ESP32
#define AUTOFORMAT_ON_FAIL true
#define LITTLEFS_BEGIN_FUNC_CALL LittleFS.begin(AUTOFORMAT_ON_FAIL, "/LittleFS", 10, "spiffs")
#endif

unsigned long currTime = 0;

bool wifiConnected = false;
bool portalRequested = false;

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
void Alarm_SendToHalCmdExec(const OnTickExtParameters *param)
{
    DEBUG_UART.println("Alarm_SendToHalCmdExec");
    const AsStringParameter* casted_param = static_cast<const AsStringParameter*>(param);
    if (casted_param != nullptr)
    {
        HAL_JSON::ZeroCopyString zcCmd(casted_param->str.c_str());
        std::string dummy;
        HAL_JSON::CommandExecutor::execute(zcCmd,dummy);
    }
}
#endif

Scheduler::NameToFunction nameToFunctionList[] = {
//   name         , onTick            , onTickExt
    {"ntp_sync"   , &Timer_SyncTime   , nullptr           }
#if defined(HAL_JSON_H_)
    ,{"halcmd"     , nullptr           , &Alarm_SendToHalCmdExec}
#endif
};

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

void setup() {
    if (Info::resetReason_is_crash(false)) {
        OTA::setup();
        System::failsafeLoop();
    }
    DEBUG_UART.begin(115200);
    DEBUG_UART.setDebugOutput(true);

    DEBUG_UART.println(F("\r\n!!!!!Start of MAIN Setup!!!!!\r\n"));
    Info::PrintHeapInfo();

    DEBUG_UART.println(Info::getResetReasonStr());

    if (LITTLEFS_BEGIN_FUNC_CALL == true) FSBrowser::fsOK = true; // this call is needed before all access to internal Flash file system

    MainConfig::begin(webserver);

#if defined(ESP32) && defined(FSBROWSER_SYNCED_WS_H_)
    if (InitSD_MMC()) FSBrowser::fsOK = true;
#endif
#if defined(USE_DISPLAY)
    init_display();
#endif
    WiFi.setSleep(false);

#ifdef WIFI_MANAGER_WRAPPER_H_
#if defined(USE_DISPLAY)
    bool connected = WiFiManagerWrapper::Setup(display);
#else
    bool connected = WiFiManagerWrapper::Setup();
#endif
#endif
    OTA::setup();

    Scheduler::setup(webserver, nameToFunctionList, sizeof(nameToFunctionList) / sizeof(nameToFunctionList[0]));

    Info::startTime = now();

    System::initWebServerHandlers(webserver);
#ifdef FSBROWSER_SYNCED_WS_H_
    FSBrowser::setup(webserver);
#else
    FSBrowser::setup(webserver);
#endif
    Info::setup(webserver);
    HeartbeatLed::setup(webserver);
#if defined(ESP32)
    System::Start_MDNS();
#endif
    
#if defined(ESP32)
    File test = SD_MMC.open("/StartTimes.log", "a", true);
    test.println(Time_ext::GetTimeAsString(now()).c_str());
    test.close();
#endif

#ifdef HAL_JSON_H_
    HAL_JSON::begin();
#endif
    webserver.begin();

    // make sure that the following are allways at the end of this function
    Info::PrintHeapInfo();
    DEBUG_UART.println(F("\r\n!!!!!End of MAIN Setup!!!!!\r\n"));
}

void loop() {
    ArduinoOTA.handle();
    HeartbeatLed::task();
    Scheduler::HandleAlarms();
#ifdef HAL_JSON_H_
    HAL_JSON::loop();
#endif
    
#if defined(ESP8266)
    MDNS.update(); // this is only required on esp8266
#endif
    
#ifdef WIFI_MANAGER_WRAPPER_H_
    WiFiManagerWrapper::Task();
#endif
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
