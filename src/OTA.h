#ifndef OTA_H
#define OTA_H

#include <Arduino.h>
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266httpUpdate.h>
#elif defined(ESP32)
#include <WiFi.h>
#include <HTTPUpdate.h>
#endif
#include <ArduinoOTA.h>

namespace OTA{

#if defined(ESP8266)
    #define DEBUG_UART Serial1
#elif defined(ESP32)
    #define DEBUG_UART Serial
#endif

    WiFiClient wifiClient;
    
    static int otaPartProcentCount = 0;
    void setup();
    void Download_Update(String url);
    void setup_PushedOTA(void);
    
    void setup()
    {
        //Pulled_check(wifiClient);
        setup_PushedOTA();
    }

    void Download_Update(String url)
    {
        Download_Update(url.c_str());
    }
    void Download_Update(const char *url)
    {
        //EEPROM.put(SPI_FLASH_SEC_SIZE, "hello");
#if defined (ESP8266)
        DEBUG_UART.println(F("checking for updates"));
        //String updateUrl = "http://espOtaServer/esp_ota/" + String(ESP.getChipId(), HEX) + ".bin";
        t_httpUpdate_return ret = ESPhttpUpdate.update(wifiClient, url);
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
#endif
    }

    void setup_PushedOTA()
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
/*
        ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {

            //DEBUG_UART.printf("Progress: %u%%\r", (progress / (total / 100)));
            //Serial1.printf("%u%%\r", (progress / (total / 100)));
            //DEBUG_UART.print("-");
            if (otaPartProcentCount < 9)
            otaPartProcentCount++;
            else
            {
            otaPartProcentCount = 0;
            DEBUG_UART.printf(" %u%%\r", (progress / (total / 100)));
            }
        });
        */

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
        //WiFi.setHostname("test");

        DEBUG_UART.println("Ready");

        DEBUG_UART.print("IP address: ");

        DEBUG_UART.println(WiFi.localIP());
    }
}
#endif