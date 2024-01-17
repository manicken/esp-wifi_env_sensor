/* 
 
*/
// basic
#include <EEPROM.h>
#include "SPI.h"
// WiFi
#include <ESP8266WiFi.h>
#include <WiFiManager.h>
// OTA
#include <ESP8266httpUpdate.h>
#include <ArduinoOTA.h>
static int otaPartProcentCount = 0;

// Amazon AWS IoT
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include "secrets/db_kitchen/secrets.h"
//#include "secrets/db_toilet/secrets.h"
//#include "secrets/db_bedroom/secrets.h"

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
#include "TCP2UART.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(128, 32, &Wire, -1); // -1 = no reset pin

#define DEBUG_UART Serial1

TCP2UART tcp2uart;

extern const char index_html[];
extern const char main_js[];
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

ESP8266WebServer server(HTTP_PORT);
HTTPClient http;
WiFiClient client;


uint32_t test = 1234567890;

uint8_t update_display = 0;

unsigned long currTime = 0;
unsigned long deltaTime_displayUpdate = 0;
unsigned long deltaTime_sendToWebUpdate = 0;


#define UPLOAD_INTERVAL_SEC (60*10)

float temp_read = 0;
float humidity_read = 0;
float temp = 0;
float humidity = 0;
float temp_old = 0;
float humidity_old = 0;

String urlApi = "";
int8_t anyChanged = 0;
DHTesp dht;

#define ONE_WIRE_BUS 12
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void printESP_info(void);
void checkForUpdates(void);
void setup_BasicOTA(void);

void setup() {
    DEBUG_UART.begin(115200);
    DEBUG_UART.println(F("\r\n!!!!!Start of MAIN Setup!!!!!\r\n"));

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
        //display.setCursor(0, 0);
        /*// Display static text
        display.println("Hello world universe");
        display.setCursor(1, 9);
        display.println("012345678901234567890");
        display.setCursor(0, 17);
        display.println("ABCDEFGHIJKLMNOPQRSTU");
        display.setCursor(0, 25);
        display.println("@!\"#-_+?%&/(){[]};:=");
        display.display();
        */
    }
    else{
        DEBUG_UART.println(F("oled init fail"));
        if (display.begin(SSD1306_SWITCHCAPVCC, 0x3D))
            DEBUG_UART.println(F("oled addr is 0x3D"));
    }

    printESP_info();
    
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
    //checkForUpdates();
    setup_BasicOTA();
    tcp2uart.begin();
    dht.setup(13, DHTesp::DHT11);

    //display.clearDisplay();
    //display.display();

    //display.display();
    temp = 0.0f;
    humidity = 0.0f;

    sensors.begin(); // one wire sensors

    DEBUG_UART.println(F("\r\n!!!!!End of MAIN Setup!!!!!\r\n"));
}

//DHT dht(13, DHT11);
float temp2_read = 0;

void loop() {
    tcp2uart.BridgeMainTask();
    ArduinoOTA.handle();

    
    currTime = millis();

    if (currTime - deltaTime_displayUpdate >= 1000) {
        deltaTime_displayUpdate = currTime;
        temp_read = dht.getTemperature();
        humidity_read = dht.getHumidity();
        sensors.requestTemperatures(); // Send the command to get temperatures
        temp2_read = sensors.getTempCByIndex(0);

        temp = temp2_read;
        humidity = humidity_read;
        
        display.setCursor(0,0);
        display.print(temp_read);
        display.print("  ");
        display.setCursor(0,8);
        display.print(temp2_read);
        display.print("  ");
        display.setCursor(0,16);
        display.print(humidity_read);
        display.print("  ");

        update_display = 1;
    }
    if (currTime - deltaTime_sendToWebUpdate >= (1000 * UPLOAD_INTERVAL_SEC)) {
        deltaTime_sendToWebUpdate = currTime;
        //temp = temp / UPLOAD_INTERVAL_SEC;
        //humidity = humidity / UPLOAD_INTERVAL_SEC;

        urlApi = "";
        //anyChanged = 0;
        //if (temp != temp_old) {
        //    temp_old = temp;
            urlApi += "&"+THINGSSPEAK_TEMP_FIELD+"=" + String(temp);
        //    anyChanged = 1;
        //}
        //if (humidity != humidity_old) {
        //    humidity_old = humidity;
            urlApi += "&"+THINGSSPEAK_HUMIDITY_FIELD+"=" + String(humidity);
        //    anyChanged = 1;
        //}
        

        //if (anyChanged == 1) {
            String url = "http://api.thingspeak.com/update?api_key="+ THINGSSPEAK_API_KEY + urlApi;
            http.begin(client, url);
            
            int httpCode = http.GET();
            if (httpCode > 0) {
                DEBUG_UART.println(F("\r\nGET request sent\r\n"));
                DEBUG_UART.println(urlApi);
            }
            else {
                DEBUG_UART.println(F("\r\nGET request FAILURE\r\n"));
                DEBUG_UART.println(urlApi);
            }
            http.end();
        //}
    }
    
    if (update_display == 1) {
        update_display = 0;
        display.display();
    }
}

void checkForUpdates(void)
{
    //EEPROM.put(SPI_FLASH_SEC_SIZE, "hello");

    DEBUG_UART.println(F("checking for updates"));
    String updateUrl = "http://espOtaServer/esp_ota/" + String(ESP.getChipId(), HEX) + ".bin";
    t_httpUpdate_return ret = ESPhttpUpdate.update(client,updateUrl.c_str());
    DEBUG_UART.print(F("HTTP_UPDATE_"));
    switch (ret) {
      case HTTP_UPDATE_FAILED:
        DEBUG_UART.println(F("FAIL Error "));
        DEBUG_UART.printf("(%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
        break;

      case HTTP_UPDATE_NO_UPDATES:
        DEBUG_UART.println(F("NO_UPDATES"));
        break;

      case HTTP_UPDATE_OK:
        DEBUG_UART.println(F("OK"));
        break;
    }
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
}

void setup_BasicOTA()
{
    ArduinoOTA.onStart([]() {
        otaPartProcentCount = 0;
        String type;

        if (ArduinoOTA.getCommand() == U_FLASH) {

          type = "sketch";

        } else { // U_SPIFFS

          type = "filesystem";

        }



        // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()

        DEBUG_UART.println(F("OTA Start\rOTA Progress: "));

      });

      ArduinoOTA.onEnd([]() {

        DEBUG_UART.println("\n100%\nOTA End");

      });

      ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {

        //DEBUG_UART.printf("Progress: %u%%\r", (progress / (total / 100)));
        //Serial1.printf("%u%%\r", (progress / (total / 100)));
        DEBUG_UART.print("-");
        if (otaPartProcentCount < 10)
          otaPartProcentCount++;
        else
        {
          otaPartProcentCount = 0;
          DEBUG_UART.printf(" %u%%\r", (progress / (total / 100)));
        }
      });

      ArduinoOTA.onError([](ota_error_t error) {
        DEBUG_UART.printf("OTA Error");
        DEBUG_UART.printf("[%u]: ", error);
        if (error == OTA_AUTH_ERROR) DEBUG_UART.println(F("Auth Failed"));
        else if (error == OTA_BEGIN_ERROR) DEBUG_UART.println(F("Begin Failed"));
        else if (error == OTA_CONNECT_ERROR) DEBUG_UART.println(F("Connect Failed"));
        else if (error == OTA_RECEIVE_ERROR) DEBUG_UART.println(F("Receive Failed"));
        else if (error == OTA_END_ERROR) DEBUG_UART.println(F("End Failed"));
      });

      ArduinoOTA.begin();

      DEBUG_UART.println("Ready");

      DEBUG_UART.print("IP address: ");

      DEBUG_UART.println(WiFi.localIP());
}
