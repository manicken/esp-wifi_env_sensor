
#include "WiFiManager.h"
#include "WifiManagerWrapper.h"



#if defined(USE_DISPLAY)
// Display
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
//#include <Fonts/FreeMono9pt7b.h>
#endif
#if defined(USE_DISPLAY)
void WiFiManager_Handler(Adafruit_SSD1306& display) {
#else
void WiFiManager_Handler() {
#endif
    WiFiManager wifiManager;
    Serial.println(F("trying to connect to saved wifi"));
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
    //display.clearDisplay();
    //display.display();
#endif
}