//#define USE_DISPLAY

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

#include "Support/Logger.h"

// OTA
#include "OTA.h"

// Amazon AWS IoT
//#include "AWS_IOT.h"


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
#include <ESP8266mDNS.h>
#elif defined(ESP32)
#include <HTTPClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#endif

#if defined(USE_DISPLAY)
// Display
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
//#include <Fonts/FreeMono9pt7b.h>
#endif

#include <WS2812FX.h>

// other addons
#if defined(ESP32)
#include <SD_MMC.h>
#endif
#include <LittleFS.h>
//#define ARDUINOJSON_ENABLE_PROGMEM 0
#include <ArduinoJson.h>
#include <TimeLib.h>
#include <TimeAlarms.h>
#include "Info.h"
#include "FSBrowser.h"
#include "RF433.h"
#include "FAN.h"
#include "Scheduler.h"
//#include "NordPoolFetcher.h"

//#include "DeviceManager.h"

#include "Time_ext.h"

#include "HearbeatLed.h"
#include "MainConfig.h"

#include "HAL_JSON/HAL_JSON.h"


#define MAIN_URLS_JSON_CMD              F("/json_cmd")

#define MAIN_URLS_FORMAT_LITTLE_FS      F("/formatLittleFs")
#define MAIN_URLS_MKDIR                 F("/mkdir")



//#include <sstream>
//#include "TCP2UART.h"

#if defined(ESP8266)
    #define DEBUG_UART Serial1
#elif defined(ESP32)
    #define DEBUG_UART Serial
#endif

//TCP2UART tcp2uart;

// QUICKFIX...See https://github.com/esp8266/Arduino/issues/263
#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))

#define WIFI_TIMEOUT 30000              // checks WiFi every ...ms. Reset after this time, if WiFi cannot reconnect.
#define HTTP_PORT 80

void connect_to_wifi(void);

void initWebServerHandlers(void);
