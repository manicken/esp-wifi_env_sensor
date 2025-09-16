
#include "System.h"
#include <Arduino.h>
#include <ArduinoOTA.h>
#include "Drivers/HearbeatLed.h" // this should not be here in final version (should only be accessible through HAL interface)

#if defined(ESP8266)
#include <ESP8266mDNS.h>
#elif defined(ESP32)
#include <ESPmDNS.h>
#endif

#include "System/MainConfig.h"

#if defined(ESP32)
#include <SD_MMC.h>
#define INIT_SDMMC_PRINT_INFO
#define INIT_SDMMC_PRINT_DIR
bool InitSD_MMC()
{
    pinMode(23, OUTPUT); // output
    digitalWrite(23, HIGH); // enable pullup on IO2(SD_D0), IO12(SD_D2)
    pinMode(2, INPUT); // input
    if (digitalRead(2) == 0) return false; // no pullup connected to GPIO2 from GPIO23
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
#if defined(ESP32)
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
#endif

void failsafeLoop()
{
    // blink rapid to alert a crash
    HeartbeatLed::HEARTBEATLED_OFF_INTERVAL = 300;
    HeartbeatLed::HEARTBEATLED_ON_INTERVAL = 300;
    //connect_to_wifi();
#ifdef WIFI_MANAGER_WRAPPER_H_
    WiFiManager_Handler(display);
#endif
    
    DEBUG_UART.begin(115200);
    DEBUG_UART.println();
    DEBUG_UART.println(F("************************************"));
    DEBUG_UART.println(F("* Now entering failsafe OTA loop.. *"));
    DEBUG_UART.println(F("************************************"));

    AsyncWebServer* asyncServer = new AsyncWebServer(80);
    asyncServer->on("/reset", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "The system will now reset and luckily go into normal mode.");
        // Small delay to ensure the response is sent before restarting
        delay(100); // NOT blocking in this context, short enough to work
#if defined(ESP32)
        esp_restart();  // Software reset
#elif defined(ESP8266)
        ESP.restart();
#endif
    });
    asyncServer->on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "The system is in OTA failsafe loop.");
    });
    asyncServer->begin();

    while (1)
    {
        ArduinoOTA.handle();
        HeartbeatLed::task();
    }
}