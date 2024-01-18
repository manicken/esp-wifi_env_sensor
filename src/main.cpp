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

void printESP_info(void);

void init_display(void);
void connect_to_wifi(void);

void setup() {
  
    DEBUG_UART.begin(115200);
    DEBUG_UART.println(F("\r\n!!!!!Start of MAIN Setup!!!!!\r\n"));

    init_display();

    printESP_info();
    connect_to_wifi();
    
    OTA::setup(wifiClient);

    tcp2uart.begin();
    dht.setup(13, DHTesp::DHT11);

    sensors.begin(); // one wire sensors

    AWS_IOT::setup_and_connect();

    DEBUG_UART.println(F("\r\n!!!!!End of MAIN Setup!!!!!\r\n"));
}



void loop() {
    tcp2uart.BridgeMainTask();
    ArduinoOTA.handle();

    
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
    if (!AWS_IOT::client.connected())
    {
        connect_to_wifi();
        AWS_IOT::setup_and_connect();
    }
    else
    {
        AWS_IOT::client.loop();
        if (millis() - deltaTime_sendToAwsIot > 5000)
        {
            deltaTime_sendToAwsIot = millis();
            AWS_IOT::publishMessage(humidity_dht, temp_ds);
        }
    }

    if (millis() - deltaTime_sendToWebUpdate >= (1000 * UPLOAD_INTERVAL_SEC)) {
        deltaTime_sendToWebUpdate = millis();

        urlApi = "";
        urlApi += "&"+String(THINGSSPEAK_TEMP_FIELD)+"=" + String(temp_ds);
        urlApi += "&"+String(THINGSSPEAK_HUMIDITY_FIELD)+"=" + String(humidity_dht);
        String url = "http://api.thingspeak.com/update?api_key="+ String(THINGSSPEAK_API_KEY) + urlApi;
        http.begin(wifiClient, url);
        
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