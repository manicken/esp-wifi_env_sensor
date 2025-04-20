/* 
 
*/
#include "main.h"
#include "UART2websocket.h"



unsigned long auto_last_change = 0;
unsigned long last_wifi_check_time = 0;

#ifdef ESP8266
ESP8266WebServer webserver(HTTP_PORT);
#define LITTLEFS_BEGIN_FUNC_CALL LittleFS.begin()
#elif ESP32
fs_WebServer webserver(HTTP_PORT);
#define AUTOFORMAT_ON_FAIL true
#define LITTLEFS_BEGIN_FUNC_CALL LittleFS.begin(AUTOFORMAT_ON_FAIL, "/LittleFS", 10, "spiffs")
#endif

unsigned long currTime = 0;

#if defined(USE_DISPLAY)
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1); // -1 = no reset pin
uint8_t update_display = 0;
unsigned long deltaTime_displayUpdate = 0;
void init_display(void);
#endif

UART2websocket uart2ws;

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

    if (ThingSpeak::canPost) {
        
        ThingSpeak::SendData();//temp_ds, humidity_dht);

    }
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

Scheduler::NameToFunction nameToFunctionList[4] = {
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
            //AWS_IOT::publishMessage(humidity_dht, temp_ds);
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
#if defined(ESP32)
#define INIT_SDMMC_PRINT_INFO
#define INIT_SDMMC_PRINT_DIR
bool InitSD_MMC()
{
    pinMode(23, OUTPUT);
    digitalWrite(23, HIGH); // enable pullup on IO2(SD_D0), IO12(SD_D2)
    pinMode(2, INPUT);
    if (digitalRead(2) == LOW) return false; // no pullup connected to GPIO2 from GPIO23
    delay(10);
    log_e("SD-card initialialize...");

    if (SD_MMC.begin("/sdcard", false, false, 20000)) {
        DEBUG_UART.println("SD-card initialized OK");
#if defined(INIT_SDMMC_PRINT_INFO)
        DEBUG_UART.print("SD card size:"); DEBUG_UART.println(SD_MMC.cardSize());
        DEBUG_UART.print("SD card type:"); 
        if (SD_MMC.cardType() == sdcard_type_t::CARD_SD) DEBUG_UART.println("CARD_SD");
        else if (SD_MMC.cardType() == sdcard_type_t::CARD_MMC) DEBUG_UART.println("CARD_MMC");
        else if (SD_MMC.cardType() == sdcard_type_t::CARD_NONE) DEBUG_UART.println("CARD_NONE");
        else if (SD_MMC.cardType() == sdcard_type_t::CARD_SDHC) DEBUG_UART.println("CARD_SDHC");
        else if (SD_MMC.cardType() == sdcard_type_t::CARD_UNKNOWN) DEBUG_UART.println("CARD_UNKNOWN");

        DEBUG_UART.print("SD card totalBytes:"); DEBUG_UART.println(SD_MMC.totalBytes());
        DEBUG_UART.print("SD card usedBytes:"); DEBUG_UART.println(SD_MMC.usedBytes());
#endif
#if defined(INIT_SDMMC_PRINT_DIR)
        FS* fileSystem = &SD_MMC;
        File root = fileSystem->open("/");

        File file;
        while (file = root.openNextFile())
        {
            DEBUG_UART.print("Name:"); DEBUG_UART.print(file.name());
            DEBUG_UART.print(", Size:"); DEBUG_UART.print(file.size());
            DEBUG_UART.print(", Dir:"); DEBUG_UART.print(file.isDirectory()?"true":"false");
            DEBUG_UART.println();
        }
#endif
        return true;
    }
    else
    {
        log_e("could not initialize/find any connected sd-card.");
        return false;
    }
}
#endif

void Start_MDNS()
{
    DEBUG_UART.println("\n\n***** STARTING mDNS service ********");
    if (MDNS.begin(MainConfig::mDNS_name.c_str())) {
        mdns_instance_name_set("ESP32 development board");
        MDNS.addService("http", "tcp", 80);
        
        if (mdns_service_add("_esp32devices", "http", "tcp", 80, NULL, 0) != ESP_OK)
            DEBUG_UART.println("Failed adding service view");
       // MDNS.addServiceTxt("http", "tcp", "path", "/");
        DEBUG_UART.println("MDNS started with name:" + MainConfig::mDNS_name);
    }
    else {
        DEBUG_UART.println("MDNS could not start");
    }
    DEBUG_UART.println("\n");
}

void failsafeLoop()
{
    // blink rapid to alert a crash
    HeartbeatLed::HEARTBEATLED_OFF_INTERVAL = 300;
    HeartbeatLed::HEARTBEATLED_ON_INTERVAL = 300;
    connect_to_wifi();
    OTA::setup();
    DEBUG_UART.begin(115200);
    DEBUG_UART.println();
    DEBUG_UART.println(F("************************************"));
    DEBUG_UART.println(F("* Now entering failsafe OTA loop.. *"));
    DEBUG_UART.println(F("************************************"));

    while (1)
    {
        ArduinoOTA.handle();
        HeartbeatLed::task();
    }
}
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
WS2812FX ws2812fx = WS2812FX(40, 22, NEO_RGB + NEO_KHZ800);
void setup() {
    if (Info::resetReason_is_crash(false)) {
        failsafeLoop();
        
    }
#if defined(ESP8266)
    FAN::init();
#endif
    DEBUG_UART.printf("free @ start:%u\n",ESP.getFreeHeap());
    DEBUG_UART.begin(115200);
    DEBUG_UART.setDebugOutput(true);
    DEBUG_UART.println(F("\r\n!!!!!Start of MAIN Setup!!!!!\r\n"));
    DEBUG_UART.println(Info::getResetReasonStr());
    if (LITTLEFS_BEGIN_FUNC_CALL == true) FSBrowser::fsOK = true; // this call is needed before all access to internal Flash file system

    MainConfig::begin(webserver);

#if defined(ESP32)
    if (InitSD_MMC()) FSBrowser::fsOK = true;
#endif
#if defined(USE_DISPLAY)
    init_display();
#endif
    WiFi.setSleep(false);
    connect_to_wifi();
    OTA::setup();
    //tcp2uart.begin();
    
    Scheduler::setup(webserver, nameToFunctionList, sizeof(nameToFunctionList) / sizeof(nameToFunctionList[0]));

    Info::startTime = now();
#if defined(AWS_IOT_H)
    AWS_IOT::setup(webserver, AWS_IOT_messageReceived);
#endif
    initWebServerHandlers();
    FSBrowser::setup(webserver);
    DeviceManager::setup(webserver);
    ThingSpeak::setup(webserver);
    Info::setup(webserver);
    webserver.begin();

#if defined(NORD_POOL_FETCHER_H)
    std::string ret = NPF::searchPatternInhtmlFromUrl();
    DEBUG_UART.println(ret.c_str());
#endif
    HeartbeatLed::setup(webserver);

    Start_MDNS();

    
    ws2812fx.init();
    ws2812fx.setBrightness(127);
    ws2812fx.setMode(12);
    ws2812fx.setSpeed(3048);
    ws2812fx.start();
    
#if defined(ESP32)
    File test = SD_MMC.open("/StartTimes.log", "a", true);
    test.println(Time_ext::GetTimeAsString(now()).c_str());
    test.close();
#endif
    uart2ws.setup();

    // make sure that the following are allways at the end of this function
    DEBUG_UART.printf("free end of setup:%u\n",ESP.getFreeHeap());
    DEBUG_UART.println(F("\r\n!!!!!End of MAIN Setup!!!!!\r\n"));
}

void loop() {
    ws2812fx.service();
    //tcp2uart.BridgeMainTask();
    ArduinoOTA.handle();
    webserver.handleClient();
    Scheduler::HandleAlarms();
    //currTime = millis();
    HeartbeatLed::task();
    uart2ws.task_loop();
#if defined(ESP8266)
    MDNS.update(); // this is only required on esp8266
#endif
#if defined(USE_DISPLAY)
    if (millis() - deltaTime_displayUpdate >= 1000) {
        deltaTime_displayUpdate = millis();
        
        //temp_dht = dht.getTemperature();
        //humidity_dht = dht.getHumidity();
        //sensors.requestTemperatures(); // Send the command to get temperatures
        
        //sensors.getTempC("00000000");
        
        //temp_ds = sensors.getTempCByIndex(0);
      
        display.setCursor(0,0);
        display.print(temp_dht);
        display.print("  ");
        display.print(humidity_dht);

        display.setCursor(0,8);
        display.print(temp_ds);
        
        update_display = 1;
        
    }
#endif
    
    if (WiFi.status() != wl_status_t:: WL_CONNECTED)
    {
        DEBUG_UART.println("WiFi connection lost.");
        wl_status_t status = WiFi.begin();
        if (status == wl_status_t::WL_CONNECTED) DEBUG_UART.println("reconnect OK");
        else DEBUG_UART.println("reconnect fail");
        
        //connect_to_wifi();
#if defined(AWS_IOT_H)
        AWS_IOT::setup_and_connect();
    }
    else if (AWS_IOT::canConnect)
    {
        if (AWS_IOT::pubSubClient.connected())
            AWS_IOT::pubSubClient.loop();
        else
            AWS_IOT::setup_and_connect();
    }
#else
    }
#endif
#if defined(USE_DISPLAY)
    if (update_display == 1) {
        update_display = 0;
        display.display();
    }
#endif
}

void initWebServerHandlers(void)
{
    webserver.on("/",  []() {
        if (LittleFS.exists(F("/index.html"))) FSBrowser::handleFileRead(F("/LittleFS/index.html"));
        else if (LittleFS.exists(F("/index.htm"))) FSBrowser::handleFileRead(F("/LittleFS/index.htm"));
        else webserver.send(404, F("text/plain"), F("404: Not Found")); // otherwise, respond with a 404 (Not Found) error
    });
    webserver.on(MAIN_URLS_JSON_CMD, HTTP_POST, [](){ webserver.send(200); }, [](){
        HTTPUpload& upload = webserver.upload();
        if (upload.status == UPLOAD_FILE_START) {
            
        } else if (upload.status == UPLOAD_FILE_WRITE) {
            
            
        } else if (upload.status == UPLOAD_FILE_END) {
            AWS_IOT_messageReceived(nullptr, upload.buf, upload.currentSize);
        }
    });
    
    webserver.on(MAIN_URLS_FORMAT_LITTLE_FS, []() {
        LITTLEFS_BEGIN_FUNC_CALL;
        if (LittleFS.format())
            webserver.send(200,"text/html", "Format OK");
        else
            webserver.send(200,"text/html", "format Fail"); 
    });
    webserver.on(MAIN_URLS_MKDIR, []() {
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
#if defined(ESP32)
    webserver.on("/sdcard_listfiles", []() {
        
        //if (SD_MMC.begin("/sdcard", true, false, 20000)) {
            File root = SD_MMC.open("/");
            if (!root) {
                webserver.send(200, F("text/plain"), "error while open sd card again");
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
            webserver.send(200, F("text/plain"), ret.c_str());
       // }else {webserver.send(200, F("text/plain"), "could not open sd card a second time");}
    });
#endif
    webserver.on("/crashTest", []() {
        webserver.send(200, F("text/plain"), "The system will now crash!!!, and luckily go into failsafe OTA upload mode.");
        int *ptr = nullptr; // Null pointer
        *ptr = 42;          // Dereference the null pointer (causes a crash)
    });
    
    webserver.onNotFound([]() {                              // If the client requests any URI
        String uri = webserver.uri();
        DEBUG_UART.println("onNotFound - hostHeader:" + webserver.hostHeader());
        bool isDir = false;
        //DEBUG_UART.println("webserver on not found:" + uri);
        if (uri.startsWith("/LittleFS") == false && uri.startsWith("/sdcard") == false)
        {
            File fileToCheck = LittleFS.open(uri);
            if (!fileToCheck) {
                webserver.send(404, F("text/plain"), F("404: Not Found")); // otherwise, respond with a 404 (Not Found) error
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

        if (!FSBrowser::handleFileRead(uri))                  // send it if it exists
            webserver.send(404, F("text/plain"), F("404: Not Found")); // otherwise, respond with a 404 (Not Found) error
    });
    
}
#if defined(USE_DISPLAY)
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
#endif

void connect_to_wifi(void)
{
    WiFiManager wifiManager;
    DEBUG_UART.println(F("trying to connect to saved wifi"));
#if defined(USE_DISPLAY)
    display.setCursor(0, 0);
    display.println(F("WiFi connecting..."));
    display.display();
#endif
    if (wifiManager.autoConnect() == true) { // using ESP.getChipId() internally
#if defined(USE_DISPLAY)
        display.setCursor(0, 9);
        display.println("OK");
        display.setCursor(0, 17);
        display.println(WiFi.localIP());
        display.display();
        delay(2000);
#endif
    } else {
#if defined(USE_DISPLAY)
        display.setCursor(0, 9);
        display.println("FAIL");
        display.display();
        delay(2000);
#endif
    }
#if defined(USE_DISPLAY)
    display.clearDisplay();
    display.display();
#endif
}

/*
#include <esp_core_dump.h>
#include <esp_log.h>
void print_core_dump() {
    esp_err_t err;
    size_t core_dump_size = 0;

    // Get core dump size
    err = esp_core_dump_image_get(&core_dump_size, NULL);
    if (err != ESP_OK || core_dump_size == 0) {
        ESP_LOGE("CORE_DUMP", "Failed to get core dump size or no core dump available.");
        return;
    }

    // Allocate buffer to read the core dump
    uint8_t *core_dump_buffer = (uint8_t *)malloc(core_dump_size);
    if (!core_dump_buffer) {
        ESP_LOGE("CORE_DUMP", "Failed to allocate buffer for core dump.");
        return;
    }

    // Retrieve core dump
    err = esp_core_dump_image_get(&core_dump_size, core_dump_buffer);
    if (err != ESP_OK) {
        ESP_LOGE("CORE_DUMP", "Failed to retrieve core dump.");
        free(core_dump_buffer);
        return;
    }

    // Print core dump to serial
    ESP_LOGI("CORE_DUMP", "Core dump size: %d bytes", core_dump_size);
    for (size_t i = 0; i < core_dump_size; i++) {
        printf("%02X", core_dump_buffer[i]);
        if ((i + 1) % 16 == 0) printf("\n");
    }

    free(core_dump_buffer);
    printf("\nCore dump printed successfully.\n");
}
*/

