/* 
 
*/
// basic
#include <EEPROM.h>
#include "SPI.h"

// WiFi
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#endif
#include <WiFiManager.h>
// OTA
#include "OTA.h"
#if defined(ESP8266)
// Amazon AWS IoT
#include "AWS_IOT.h"
#endif

// Thingspeak
#include "ThingSpeak.h"

// sensors
#include <DHTesp.h>
#include <OneWire.h>
#include <DallasTemperature.h>


// HTTP stuff
#if defined(ESP8266)
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#elif defined(ESP32)
#include <HTTPClient.h>
#include <WebServer.h>
#endif

// Display
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
//#include <Fonts/FreeMono9pt7b.h>

// other addons
#include <LittleFS.h>
//#define ARDUINOJSON_ENABLE_PROGMEM 0
#include <ArduinoJson.h>
#include <TimeLib.h>
#include <TimeAlarms.h>
#include "FSBrowser.h"
#include "RF433.h"
#include "FAN.h"
#include "TimeAlarmsFromJson.h"
//#include "NordPoolFetcher.h"

#include "GPIO_manager.h"

//#include <sstream>
//#include "TCP2UART.h"

// the following are not used when having config and files on internal filesystem
//#include "secrets/db_kitchen/secrets.h"
//#include "secrets/db_toilet/secrets.h"
//#include "secrets/db_bedroom/secrets.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(128, 32, &Wire, -1); // -1 = no reset pin

#define DEBUG_UART Serial

//TCP2UART tcp2uart;

// QUICKFIX...See https://github.com/esp8266/Arduino/issues/263
#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))

#define LED_PIN 5                       // 0 = GPIO0, 2=GPIO2
#define LED_COUNT 50

#define WIFI_TIMEOUT 30000              // checks WiFi every ...ms. Reset after this time, if WiFi cannot reconnect.
#define HTTP_PORT 80

#define DOGM_LCD_CS 0
#define DOGM_LCD_RS 5

#define PULSE_INPUT_A 12
#define PULSE_INPUT_B 13

unsigned long auto_last_change = 0;
unsigned long last_wifi_check_time = 0;

#ifdef ESP8266
ESP8266WebServer webserver(HTTP_PORT);
#define LITTLEFS_BEGIN_FUNC_CALL LittleFS.begin()
#elif ESP32
fs_WebServer webserver(HTTP_PORT);
#define LITTLEFS_BEGIN_FUNC_CALL LittleFS.begin(false, "", 10, "spiffs")
#endif

uint32_t test = 1234567890;

uint8_t update_display = 0;

unsigned long currTime = 0;
unsigned long deltaTime_displayUpdate = 0;
unsigned long deltaTime_sendToWebUpdate = 0;
unsigned long deltaTime_sendToAwsIot = 0;

DHTesp dht;
float temp_dht = 0;
float humidity_dht = 0;

#define ONE_WIRE_BUS 12
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
float temp_ds = 0;

time_t startTime = 0;

void init_display(void);
void connect_to_wifi(void);
void printESP_info(void);
void srv_handle_info(void);
const char* getResetReason(void);
void initWebServerHandlers(void);
std::string GetTimeAsString(time_t time);

void Timer_SyncTime() {
    DEBUG_UART.println("Timer_SyncTime");
    NTP::NTPConnect();
    tmElements_t now2;
    breakTime(time(nullptr), now2);
    int year = (int)now2.Year + 1970;
    setTime(now2.Hour+1, now2.Minute, now2.Second, now2.Day, now2.Month, year);
}

void Timer_SendEnvData()
{
    DEBUG_UART.println("Timer_SendEnvData");
    if (ThingSpeak::canPost)
        ThingSpeak::SendData(temp_ds, humidity_dht);
}
void Alarm_SetFanSpeed(const OnTickExtParameters *param)
{
    DEBUG_UART.println("\nAlarm_SetFanSpeed");
    const AsStringParameter* casted_param = static_cast<const AsStringParameter*>(param);
    if (casted_param != nullptr)
    {
        FAN::DecodeFromJSON(casted_param->jsonStr);
    }
}
void Alarm_SendToRF433(const OnTickExtParameters *param)
{
    DEBUG_UART.println("Alarm_SendToRF433");
    const AsStringParameter* casted_param = static_cast<const AsStringParameter*>(param);
    if (casted_param != nullptr)
    {
        RF433::DecodeFromJSON(casted_param->jsonStr);
    }
}

TimeAlarmsFromJson::NameToFunction nameToFunctionList[4] = {
//   name         , onTick            , onTickExt
    {"ntp_sync"   , &Timer_SyncTime   , nullptr           },
    {"sendEnvData", &Timer_SendEnvData, nullptr           },
    {"fan"        , nullptr           , &Alarm_SetFanSpeed},
    {"rf433"      , nullptr           , &Alarm_SendToRF433}
};


void AWS_IOT_messageReceived(char *topic, byte *payload, unsigned int length)
{
    DEBUG_UART.print("Received [");
    DEBUG_UART.print(topic);
    DEBUG_UART.print("]: ");
    for (unsigned int i = 0; i < length; i++)
    {
        DEBUG_UART.print((char)payload[i]);
    }
    DEBUG_UART.println();
    DynamicJsonDocument jsonDoc(256);

    deserializeJson(jsonDoc, payload);


    if (jsonDoc.containsKey("cmd"))
    {
        std::string cmd = (std::string)jsonDoc["cmd"].as<std::string>();
        if (cmd == "sendEnvData")
        {
#if defined(ESP8266)
            DEBUG_UART.println("sending to AWS IOT");
            AWS_IOT::publishMessage(humidity_dht, temp_ds);
#endif
        }
        else if (cmd == "RF433")
        {
            JsonVariant jsonVariant = jsonDoc.as<JsonVariant>();
            RF433::DecodeFromJSON(jsonVariant);
        }
        else if (cmd == "FAN")
        {
            JsonVariant jsonVariant = jsonDoc.as<JsonVariant>();
            FAN::DecodeFromJSON(jsonVariant);
        }
        else if (cmd == "OTA_update")
        {
            if (!jsonDoc.containsKey("url")) return;

            std::string url = (std::string)jsonDoc["url"].as<std::string>();

            DEBUG_UART.printf("starting OTA from %s\n", url.c_str());
            
            OTA::Download_Update(url.c_str());
        }
    }
}

void setup() {

    FAN::init();
DEBUG_UART.printf("free @ start:%u\n",ESP.getFreeHeap());
    DEBUG_UART.begin(115200);
    DEBUG_UART.setDebugOutput(true);
    DEBUG_UART.println(F("\r\n!!!!!Start of MAIN Setup!!!!!\r\n"));
    DEBUG_UART.println(getResetReason());
    LITTLEFS_BEGIN_FUNC_CALL;

    init_display();
    connect_to_wifi();
    OTA::setup();
    //tcp2uart.begin();
    dht.setup(13, DHTesp::DHT11);
    sensors.begin(); // one wire sensors
    RF433::init(14);
    
    NTP::NTPConnect();
    tmElements_t now2;
    breakTime(time(nullptr), now2);
    int year = (int)now2.Year + 1970;
    setTime(now2.Hour+1, now2.Minute, now2.Second, now2.Day, now2.Month, year);
    startTime = now();

    size_t nameToFunctionList_Count = sizeof(nameToFunctionList) / sizeof(nameToFunctionList[0]);
    DEBUG_UART.printf("nameToFunctionList_Count:%d\r\n", nameToFunctionList_Count);
    TimeAlarmsFromJson::SetFunctionTable(nameToFunctionList, 3);
    TimeAlarmsFromJson::LoadJson("/schedule/list.json");
    
#if defined(ESP8266)
    if (AWS_IOT::setup_readFiles()) {
        AWS_IOT::setup_and_connect(AWS_IOT_messageReceived);
    }
    else
        DEBUG_UART.println(F("AWS_IOT error cannot setup AWS IoT without the cert and key files!!!"));
#endif

    ThingSpeak::loadSettings();
    initWebServerHandlers();
    FSBrowser::setup(webserver);
    GPIO_manager::setup(webserver);
    webserver.begin();
#if defined(ESP8266)
    //std::string ret = NPF::searchPatternInhtmlFromUrl();
    // DEBUG_UART.println(ret.c_str());
#endif
DEBUG_UART.printf("free end of setup:%u\n",ESP.getFreeHeap());
    DEBUG_UART.println(F("\r\n!!!!!End of MAIN Setup!!!!!\r\n"));
}

void loop() {
    //tcp2uart.BridgeMainTask();
    
    ArduinoOTA.handle();
    webserver.handleClient();
    TimeAlarmsFromJson::HandleAlarms();
    currTime = millis();

    if (millis() - deltaTime_displayUpdate >= 1000) {
        deltaTime_displayUpdate = millis();
        temp_dht = dht.getTemperature();
        humidity_dht = dht.getHumidity();
        sensors.requestTemperatures(); // Send the command to get temperatures
        
        //sensors.getTempC("00000000");
        
        temp_ds = sensors.getTempCByIndex(0);
      
        display.setCursor(0,0);
        display.print(temp_dht);
        display.print("  ");
        display.print(humidity_dht);

        display.setCursor(0,8);
        display.print(temp_ds);
        
        update_display = 1;
    }
    
    if (WiFi.status() != WL_CONNECTED)
    {
        connect_to_wifi();
#if defined(ESP8266)
        AWS_IOT::setup_and_connect(AWS_IOT_messageReceived);
    }
    else if (AWS_IOT::canConnect)
    {
        if (!AWS_IOT::pubSubClient.connected())
            AWS_IOT::setup_and_connect(AWS_IOT_messageReceived);
        else
            AWS_IOT::pubSubClient.loop();
        
    }
#else
    }
#endif

    if (update_display == 1) {
        update_display = 0;
        display.display();
    }
}


void initWebServerHandlers(void)
{

    webserver.on("/",  []() {
        if (LittleFS.exists(F("index.html"))) FSBrowser::handleFileRead(F("index.html"));
        else if (LittleFS.exists(F("index.htm"))) FSBrowser::handleFileRead(F("index.htm"));
        else webserver.send(404, F("text/plain"), F("404: Not Found")); // otherwise, respond with a 404 (Not Found) error
    });
    webserver.on(F("/json_cmd"), HTTP_POST, [](){ webserver.send(200); }, [](){
        HTTPUpload& upload = webserver.upload();
        if (upload.status == UPLOAD_FILE_START) {
            
        } else if (upload.status == UPLOAD_FILE_WRITE) {
            
            
        } else if (upload.status == UPLOAD_FILE_END) {
            AWS_IOT_messageReceived(nullptr, upload.buf, upload.currentSize);
        }
    });
    webserver.on(F("/info"), srv_handle_info);
    webserver.on(F("/formatLittleFs"), []() {
        LITTLEFS_BEGIN_FUNC_CALL;
        if (LittleFS.format())
            webserver.send(200,"text/html", "Format OK");
        else
            webserver.send(200,"text/html", "format Fail"); 
    });
    webserver.on(F("/mkdir"), []() {
        if (!webserver.hasArg("dir")) { webserver.send(200,"text/html", "Error: dir argument missing"); }
        else
        {
            String path = webserver.arg("dir");
            
            if (LittleFS.mkdir(path.c_str()) == false) {
                webserver.send(200,"text/html", "Error: could not create dir:" + path);
            }
            else
            {
                webserver.send(200,"text/html", "create new dir OK");
            }

        }

    });
    webserver.on(F("/oneWire/list"), []() {
        uint8_t pin = 32;
        if (webserver.hasArg("pin"))
            pin = std::stoi(webserver.arg("pin").c_str());
        OneWire _1wire;
        _1wire.begin(pin);
        byte i = 0;
        byte done = 0;
        byte addr[8];
        String returnStr;
        char hexString[3];

        while(!done)
        {
            if (_1wire.search(addr) != 1)
            {
                returnStr.concat("<br>No more addresses.<br>");
                _1wire.reset_search();
                done = 1;
            }
            else
            {
                returnStr.concat("<br>Unique ID = ");
                for( i = 0; i < 7; i++) 
                {
                    sprintf(hexString, "%02X", addr[i]);
                    returnStr.concat(hexString);
                    returnStr.concat(":");
                }
                sprintf(hexString, "%02X", addr[7]);
                    returnStr.concat(hexString);
                returnStr.concat("<br>");
            }
        }
        webserver.send(200,F("text/html"), returnStr);
    });
#if defined(ESP8266)
    webserver.on(F("/aws_iot/refresh"), []() {
        if (AWS_IOT::setup_readFiles()) {
            webserver.send(200,F("text/plain"), F("AWS_IOT setup_readFiles OK"));
            AWS_IOT::setup_and_connect(AWS_IOT_messageReceived);
            
        }
        else
        {
            webserver.send(200,F("text/plain"), F("AWS_IOT setup_readFiles Fail"));
        }
    });
#endif
    webserver.on(F("/thingspeak/refresh"), []() {
        if (ThingSpeak::loadSettings())
            webserver.send(200,F("text/plain"), F("Thingspeak loadSettings OK"));
        else
            webserver.send(200,F("text/plain"), F("Thingspeak loadSettings error"));
    });
    webserver.on(F("/schedule/refresh"), []() {
        if (TimeAlarmsFromJson::LoadJson(F("/schedule/list.json")))
            webserver.send(200,F("text/plain"), F("schedule load json OK"));
        else
            webserver.send(200,F("text/plain"), F("schedule load json error"));
    });
    webserver.on(F("/esp/free_heap"), []() {
        std::string ret = "Free Heap:" + std::to_string(ESP.getFreeHeap());
#if defined(ESP8266)
        ret += ", Fragmentation:" + std::to_string(ESP.getHeapFragmentation());
#endif
        webserver.send(200,F("text/plain"), ret.c_str());
    });
    webserver.on(F("/esp/last_reset_reason"), []() {
        std::string resetInfo = "Last Reset at: " + GetTimeAsString(startTime);
        resetInfo += "\nReason: " + std::string(getResetReason());
        
        webserver.send(200, F("text/plain"), resetInfo.c_str());
    });
    webserver.on(F("/schedule/getMaxNumberOfAlarms"), []() {
        std::string ret = std::to_string(dtNBR_ALARMS);
        webserver.send(200, F("text/plain"), ret.c_str());
    });
    webserver.on(F("/schedule/getTime"), []() {
        std::string nowstr = GetTimeAsString(now());
        webserver.send(200,F("text/plain"), nowstr.c_str());
    });
    webserver.on(F("/schedule/getFunctionNames"), []() {
        int item_Count = sizeof(nameToFunctionList) / sizeof(nameToFunctionList[0]);

        std::string jsonStr = "{";

        for (int i=0;i<item_Count;i++) {
            jsonStr += "\"" +  nameToFunctionList[i].name + "\":\"" + ((nameToFunctionList[i].onTickExt!=nullptr)?"p":"") + "\"";
            if (i < (item_Count-1)) jsonStr += ",";
        }
        jsonStr += "}";
        webserver.send(200,F("text/plain"), jsonStr.c_str());
    });
    webserver.on(F("/schedule/getShortDows"), []() {
        std::string ret = TimeAlarmsFromJson::GetShortFormDowListAsJson();
        webserver.send(200,F("text/plain"), ret.c_str());
    });
    webserver.onNotFound([]() {                              // If the client requests any URI
        String uri = webserver.uri();
        if (uri.indexOf('.') == -1) // if it's a folder, try to find index.htm or index.html
        {
            if (uri.endsWith("/") == false)
                uri += "/";
            if (LittleFS.exists(uri + "/index.html"))
                uri += "index.html";
            else if (LittleFS.exists(uri + "/index.htm"))
                uri += "index.htm";
        }
        if (!FSBrowser::handleFileRead(uri))                  // send it if it exists
            webserver.send(404, F("text/plain"), F("404: Not Found")); // otherwise, respond with a 404 (Not Found) error
    });
}

void init_display(void)
{
    if (display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    {
        delay(2000);
        display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
        delay(2000);
        display.clearDisplay();
        display.display();
        //display.setFont(&FreeMono9pt7b);
        display.setTextSize(1);
        display.setTextColor(WHITE, BLACK);

    }
    else{
        //DEBUG_UART.println(F("oled init fail"));
        display.begin(SSD1306_SWITCHCAPVCC, 0x3D);
        //if (display.begin(SSD1306_SWITCHCAPVCC, 0x3D))
            //DEBUG_UART.println(F("oled addr is 0x3D"));
    }
}

void connect_to_wifi(void)
{
    WiFiManager wifiManager;
    DEBUG_UART.println(F("trying to connect to saved wifi"));
    display.setCursor(0, 0);
    display.println(F("WiFi connecting..."));
    display.display();
    if (wifiManager.autoConnect() == true) { // using ESP.getChipId() internally
        display.setCursor(0, 9);
        display.println("OK");
        display.setCursor(0, 17);
        display.println(WiFi.localIP());
        display.display();
        delay(2000);
    } else {
        display.setCursor(0, 9);
        display.println("FAIL");
        display.display();
        delay(2000);
    }
    display.clearDisplay();
    display.display();
}

/*
// called from setup() function
void printESP_info(void) { 
    uint32_t realSize = ESP.getFlashChipRealSize();
    uint32_t ideSize = ESP.getFlashChipSize();
    FlashMode_t ideMode = ESP.getFlashChipMode();

    DEBUG_UART.print(F("Flash real id:   ")); DEBUG_UART.printf("%08X\r\n", ESP.getFlashChipId());
    DEBUG_UART.print(F("Flash real size: ")); DEBUG_UART.printf("%u 0\r\n\r\n", realSize);

    DEBUG_UART.print(F("Flash ide  size: ")); DEBUG_UART.printf("%u\r\n", ideSize);
    DEBUG_UART.print(F("Flash ide speed: ")); DEBUG_UART.printf("%u\r\n", ESP.getFlashChipSpeed());
    DEBUG_UART.print(F("Flash ide mode:  ")); DEBUG_UART.printf("%s\r\n", (ideMode == FM_QIO ? "QIO" : ideMode == FM_QOUT ? "QOUT" : ideMode == FM_DIO ? "DIO" : ideMode == FM_DOUT ? "DOUT" : "UNKNOWN"));

    if(ideSize != realSize)
    {
        DEBUG_UART.println(F("Flash Chip configuration wrong!\r\n"));
    }
    else
    {
        DEBUG_UART.println(F("Flash Chip configuration ok.\r\n"));
    }
    DEBUG_UART.printf(" ESP8266 Chip id = %08X\n", ESP.getChipId());
    DEBUG_UART.println();
    DEBUG_UART.println();
}*/

/*
    enum rst_reason {
    REASON_DEFAULT_RST      = 0,    normal startup by power on 
    REASON_WDT_RST          = 1,    hardware watch dog reset 
    REASON_EXCEPTION_RST    = 2,    exception reset, GPIO status won’t change 
    REASON_SOFT_WDT_RST     = 3,    software watch dog reset, GPIO status won’t change 
    REASON_SOFT_RESTART     = 4,    software restart ,system_restart , GPIO status won’t change 
    REASON_DEEP_SLEEP_AWAKE = 5,    wake up from deep-sleep 
    REASON_EXT_SYS_RST      = 6     external system reset
};*/
const char* getResetReason()
{
#if defined(ESP8266)
    rst_info *info = system_get_rst_info();
    uint32 reason = info->reason;
    if (reason == rst_reason::REASON_DEFAULT_RST)
        return "normal startup by power on";
    else if (reason == rst_reason::REASON_WDT_RST)
        return "hardware watch dog reset";
    else if (reason == rst_reason::REASON_EXCEPTION_RST)
        return "exception reset";
    else if (reason == rst_reason::REASON_SOFT_WDT_RST)
        return "software watch dog reset";
    else if (reason == rst_reason::REASON_SOFT_RESTART)
        return "software restart/system_restart";
    else if (reason == rst_reason::REASON_DEEP_SLEEP_AWAKE)
        return "wake up from deep-sleep";
    else if (reason == rst_reason::REASON_EXT_SYS_RST)
        return "external system reset";
    else
#endif
        return "undefined reset cause";
}

void listDir(const char *dirname, uint8_t levels) {
  DEBUG_UART.printf("Listing directory: %s\r\n", dirname);

  File root = LittleFS.open(dirname);
  if (!root) {
    DEBUG_UART.println("- failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    DEBUG_UART.println(" - not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      DEBUG_UART.print("  DIR : ");
      DEBUG_UART.println(file.name());
      if (levels) {
        listDir(file.path(), levels - 1);
      }
    } else {
      DEBUG_UART.print("  FILE: ");
      DEBUG_UART.print(file.name());
      DEBUG_UART.print("\tSIZE: ");
      DEBUG_UART.println(file.size());
    }
    file = root.openNextFile();
  }
  DEBUG_UART.println();
}

void srv_handle_info()
{
    uint32_t ideSize = ESP.getFlashChipSize();
#if defined(ESP8266)
    uint32_t realSize = ESP.getFlashChipRealSize();
#else
    uint32_t realSize = ideSize;
#endif
    FlashMode_t ideMode = ESP.getFlashChipMode();
    String srv_return_msg = "";

    srv_return_msg.concat(F("<!DOCTYPE html PUBLIC\"ISO/IEC 15445:2000//DTD HTML//EN\"><html><head><title></title></head><body>"));
#if defined(ESP8266)
    srv_return_msg.concat(F("Flash real id:   ")); srv_return_msg.concat(ESP.getFlashChipId());
#endif
    srv_return_msg.concat(F("<br>Flash real size: ")); srv_return_msg.concat(realSize);

    srv_return_msg.concat(F("<br>Flash ide  size: ")); srv_return_msg.concat(ideSize);
    srv_return_msg.concat(F("<br>Flash ide speed: ")); srv_return_msg.concat(ESP.getFlashChipSpeed());
    srv_return_msg.concat(F("<br>Flash ide mode:  ")); srv_return_msg.concat((ideMode == FM_QIO ? "QIO" : ideMode == FM_QOUT ? "QOUT" : ideMode == FM_DIO ? "DIO" : ideMode == FM_DOUT ? "DOUT" : "UNKNOWN"));
    if(ideSize != realSize)
    {
        srv_return_msg.concat(F("<br>Flash Chip configuration wrong!\r\n"));
    }
    else
    {
        srv_return_msg.concat(F("<br>Flash Chip configuration ok.\r\n"));
    }
#if defined(ESP8266)
    srv_return_msg.concat(F("<br> ESP8266 Chip id = ")); srv_return_msg.concat(ESP.getChipId());
#endif
    srv_return_msg.concat(F("<br><br>"));
    if (LITTLEFS_BEGIN_FUNC_CALL) {
        srv_return_msg.concat(F("<br>LittleFS mounted OK"));
#if defined(ESP8266)
        FSInfo fsi;
        if (LittleFS.info(fsi)) {
            srv_return_msg.concat(F("<br>LittleFS blocksize = ")); srv_return_msg.concat(fsi.blockSize); 
            srv_return_msg.concat(F("<br>LittleFS maxOpenFiles = ")); srv_return_msg.concat(fsi.maxOpenFiles); 
            srv_return_msg.concat(F("<br>LittleFS maxPathLength = ")); srv_return_msg.concat(fsi.maxPathLength); 
            srv_return_msg.concat(F("<br>LittleFS pageSize = ")); srv_return_msg.concat(fsi.pageSize); 
            srv_return_msg.concat(F("<br>LittleFS totalBytes = ")); srv_return_msg.concat(fsi.totalBytes); 
            srv_return_msg.concat(F("<br>LittleFS usedBytes = ")); srv_return_msg.concat(fsi.usedBytes); 
        }
        else
#elif defined(ESP32)
        srv_return_msg.concat(F("<br>LittleFS totalBytes = ")); srv_return_msg.concat(LittleFS.totalBytes());
        srv_return_msg.concat(F("<br>LittleFS usedBytes = ")); srv_return_msg.concat(LittleFS.usedBytes()); 
#else
            srv_return_msg.concat(F("<br>LittleFS info not implemented"));
#endif

        String str = "<br><br>Files:<br>";
        
#if defined(ESP8266)
        Dir dir = LittleFS.openDir("/");
        
        while (dir.next()) {
            str += dir.fileName();
            str += " / ";
            str += dir.fileSize();
            str += "<br>";
        }
#elif defined(ESP32)
    listDir("/", 10);
    File root = LittleFS.open("/");
    File file = root.openNextFile();

    while (file) {
        str.concat(file.path());
        str.concat(file.name());
        str.concat(" / ");
        str.concat(file.size());
        if (file.isDirectory())
            str.concat(" (dir) ");
        str.concat("<br>");
        file = root.openNextFile(); 
    }
    
#endif
        srv_return_msg.concat(str);
    }
    else
        srv_return_msg.concat(F("<br>LittleFS Fail to mount"));

    

    srv_return_msg.concat(F("</body></html>"));
    webserver.send(200, "text/html", srv_return_msg);
    //server.sendContent(srv_return_msg);

    //server.sendContent("");
}

std::string formatNumber(int num) {
    return (num < 10) ? "0" + std::to_string(num) : std::to_string(num);
}

std::string formatTime(int hour, int minute, int second) {
    return formatNumber(hour) + ":" + formatNumber(minute) + ":" + formatNumber(second);
}

std::string GetTimeAsString(time_t time)
{
    tmElements_t now2;
    breakTime(time, now2);
    std::string nowstr = std::to_string(now2.Year + 1970) + "-" +
                    std::to_string(now2.Month) + "-" +
                    std::to_string(now2.Day) + " " +
                    formatTime(now2.Hour, now2.Minute, now2.Second);
                    
    return nowstr;
}