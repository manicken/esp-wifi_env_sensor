/* 
 
*/
// basic
#include <EEPROM.h>
#include "SPI.h"

// WiFi
#include <ESP8266WiFi.h>
#include <WiFiManager.h>
// OTA
#include "OTA.h"
// Amazon AWS IoT
#include "AWS_IOT.h"
// Thingspeak
#include "ThingSpeak.h"

// sensors
#include <DHTesp.h>
#include <OneWire.h>
#include <DallasTemperature.h>


// HTTP stuff
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>

// Display
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
//#include <Fonts/FreeMono9pt7b.h>

// other addons
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "FSBrowser.h"
#include "TCP2UART.h"

// the following are not used when having config and files on internal filesystem
#include "secrets/db_kitchen/secrets.h"
//#include "secrets/db_toilet/secrets.h"
//#include "secrets/db_bedroom/secrets.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(128, 32, &Wire, -1); // -1 = no reset pin

#define DEBUG_UART Serial1

TCP2UART tcp2uart;

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

ESP8266WebServer webserver(HTTP_PORT);

WiFiClient wifiClient;

uint32_t test = 1234567890;

uint8_t update_display = 0;

unsigned long currTime = 0;
unsigned long deltaTime_displayUpdate = 0;
unsigned long deltaTime_sendToWebUpdate = 0;
unsigned long deltaTime_sendToAwsIot = 0;


#define UPLOAD_INTERVAL_SEC (60*10)

String urlApi = "";
int8_t anyChanged = 0;

DHTesp dht;
float temp_dht = 0;
float humidity_dht = 0;

#define ONE_WIRE_BUS 12
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
float temp_ds = 0;

StaticJsonDocument<200> jsonDoc;

char jsonBuffer[512];



void init_display(void);
void connect_to_wifi(void);
void printESP_info(void);
void srv_handle_info(void);



void AWS_IOT_messageReceived(char *topic, byte *payload, unsigned int length)
{
    DEBUG_UART.print("Received [");
    DEBUG_UART.print(topic);
    DEBUG_UART.print("]: ");
    for (int i = 0; i < length; i++)
    {
        DEBUG_UART.print((char)payload[i]);
    }
    DEBUG_UART.println();

    deserializeJson(jsonDoc, payload);

    if (jsonDoc.containsKey("cmd"))
    {
        std::string cmd = (std::string)jsonDoc["cmd"];
        if (cmd == "sendEnvData")
        {
            DEBUG_UART.println("sending to AWS IOT");
            AWS_IOT::publishMessage(humidity_dht, temp_ds);
        }
        else if (cmd == "OTA_update")
        {
            if (jsonDoc.containsKey("path"))
            {
                std::string url = (std::string)jsonDoc["url"];

                DEBUG_UART.printf("starting OTA from %s\n", url.c_str());
                
                OTA::Download_Update(wifiClient, url.c_str());
            }
        }
    }
}

void setup() {
  
   
    DEBUG_UART.begin(115200);
    DEBUG_UART.setDebugOutput(true);
    DEBUG_UART.println(F("\r\n!!!!!Start of MAIN Setup!!!!!\r\n"));
    DEBUG_UART.print("Init LittleFS ");
    if ( LittleFS.begin())
        DEBUG_UART.println("[OK]");
    else
        DEBUG_UART.println("[FAIL]");

    init_display();

    //printESP_info();
    connect_to_wifi();
    
    OTA::setup(wifiClient);

    tcp2uart.begin();
    dht.setup(13, DHTesp::DHT11);

    sensors.begin(); // one wire sensors

    if (AWS_IOT::setup_readFiles())
        AWS_IOT::setup_and_connect(AWS_IOT_messageReceived);
    else
        DEBUG_UART.println("AWS_IOT error cannot setup AWS IoT without the cert and key files!!!");

    ThingSpeak::loadSettings();

    webserver.on("/",  []() {
        if (LittleFS.exists("index.html")) FSBrowser::handleFileRead("index.html");
        else if (LittleFS.exists("index.htm")) FSBrowser::handleFileRead("index.htm");
        else webserver.send(404, "text/plain", "404: Not Found"); // otherwise, respond with a 404 (Not Found) error
    });
    webserver.on("/info", srv_handle_info);
    webserver.on("/formatLittleFs", []() { if (LittleFS.format()) webserver.send(200,"text/html", "Format OK"); else webserver.send(200,"text/html", "format Fail"); });
    webserver.on("/aws_iot/refresh", []() {
        if (AWS_IOT::setup_readFiles()) {
            webserver.send(200,"text/html", "AWS_IOT setup_readFiles OK");
            AWS_IOT::setup_and_connect(AWS_IOT_messageReceived);
            
        }
        else
        {
            webserver.send(200,"text/html", "AWS_IOT setup_readFiles Fail");
        }
        });
    webserver.on("/thingspeak/refresh", []() {
        if (ThingSpeak::loadSettings())
            webserver.send(200,"text/html", "Thingspeak loadSettings OK");
        else
            webserver.send(200,"text/html", "Thingspeak loadSettings error");
    });
    webserver.onNotFound([]() {                              // If the client requests any URI
        if (!FSBrowser::handleFileRead(webserver.uri()))                  // send it if it exists

        webserver.send(404, "text/plain", "404: Not Found"); // otherwise, respond with a 404 (Not Found) error
    });

    FSBrowser::setup(webserver);

    webserver.begin();
    

    DEBUG_UART.println(F("\r\n!!!!!End of MAIN Setup!!!!!\r\n"));


}

void loop() {
    tcp2uart.BridgeMainTask();
    ArduinoOTA.handle();
    webserver.handleClient();
    
    currTime = millis();

    if (millis() - deltaTime_displayUpdate >= 1000) {
        deltaTime_displayUpdate = millis();
        temp_dht = dht.getTemperature();
        humidity_dht = dht.getHumidity();
        sensors.requestTemperatures(); // Send the command to get temperatures
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
        AWS_IOT::setup_and_connect(AWS_IOT_messageReceived);
    }
    else if (AWS_IOT::canConnect)
    {
        if (!AWS_IOT::pubSubClient.connected())
            AWS_IOT::setup_and_connect(AWS_IOT_messageReceived);
        else
            AWS_IOT::pubSubClient.loop();
        
    }


    if (millis() - deltaTime_sendToWebUpdate >= (1000 * ThingSpeak::update_rate_sec)) {
        deltaTime_sendToWebUpdate = millis();
        if (ThingSpeak::canPost)
            ThingSpeak::SendData(temp_ds, humidity_dht);
    }
    
    if (update_display == 1) {
        update_display = 0;
        display.display();
    }
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
        DEBUG_UART.println(F("oled init fail"));
        if (display.begin(SSD1306_SWITCHCAPVCC, 0x3D))
            DEBUG_UART.println(F("oled addr is 0x3D"));
    }
}

void connect_to_wifi(void)
{
    WiFiManager wifiManager;
    DEBUG_UART.println(F("trying to connect to saved wifi"));
    display.setCursor(0, 0);
    display.println("WiFi connecting...");
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
}

void srv_handle_info()
{
    uint32_t realSize = ESP.getFlashChipRealSize();
    uint32_t ideSize = ESP.getFlashChipSize();
    FlashMode_t ideMode = ESP.getFlashChipMode();
    String srv_return_msg = "";

    srv_return_msg.concat(F("<!DOCTYPE html PUBLIC\"ISO/IEC 15445:2000//DTD HTML//EN\"><html><head><title></title></head><body>"));

    srv_return_msg.concat(F("Flash real id:   ")); srv_return_msg.concat(ESP.getFlashChipId());
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
    srv_return_msg.concat(F("<br> ESP8266 Chip id = ")); srv_return_msg.concat(ESP.getChipId());
    srv_return_msg.concat(F("<br><br>"));
    if (LittleFS.begin()) {
        srv_return_msg.concat(F("<br>LittleFS mounted OK"));
        FSInfo fsi;
        if (LittleFS.info(fsi)) {
            srv_return_msg.concat(F("<br>LittleFS blocksize = ")); srv_return_msg.concat(fsi.blockSize); 
            srv_return_msg.concat(F("<br>LittleFS maxOpenFiles = ")); srv_return_msg.concat(fsi.maxOpenFiles); 
            srv_return_msg.concat(F("<br>LittleFS maxPathLength = ")); srv_return_msg.concat(fsi.maxPathLength); 
            srv_return_msg.concat(F("<br>LittleFS pageSize = ")); srv_return_msg.concat(fsi.pageSize); 
            srv_return_msg.concat(F("<br>LittleFS totalBytes = ")); srv_return_msg.concat(fsi.totalBytes); 
            srv_return_msg.concat(F("<br>LittleFS usedBytes = ")); srv_return_msg.concat(fsi.usedBytes); 
        }
        else srv_return_msg.concat(F("<br>LittleFS info not implemented"));

        String str = "<br><br>Files:<br>";
        Dir dir = LittleFS.openDir("/");
        while (dir.next()) {
            str += dir.fileName();
            str += " / ";
            str += dir.fileSize();
            str += "<br>";
        }
        srv_return_msg.concat(str);
    }
    else
        srv_return_msg.concat(F("<br>LittleFS Fail to mount"));

    

    srv_return_msg.concat(F("</body></html>"));
    webserver.send(200, "text/html", srv_return_msg);
    //server.sendContent(srv_return_msg);

    //server.sendContent("");
}