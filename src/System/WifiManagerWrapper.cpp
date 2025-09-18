
#include "WiFiManager.h"
#include "WifiManagerWrapper.h"

#if defined(USE_DISPLAY)
// Display
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#endif

namespace WiFiManagerWrapper {

    WiFiManager wifiManager;
    bool wifiConnected = false;
    bool portalRequested = false;

#if defined(USE_DISPLAY)
    void Setup(Adafruit_SSD1306& display) {
#else
    void Setup() {
#endif

        Serial.println(F("trying to connect to saved wifi"));
#if defined(USE_DISPLAY)
        display.setCursor(0, 0);
        display.println(F("WiFi connecting..."));
        display.display();
#endif
        // Try connect to saved WiFi, portal starts if it fails
        wifiManager.setConfigPortalTimeout(180); // 3 min to run the portal otherwise it will continue
        // autoConnect is using ESP.getChipId() internally
        wifiManager.autoConnect();
        wifiConnected = (WiFi.status() == WL_CONNECTED);

        if (wifiConnected) {
#if defined(USE_DISPLAY)
            display.setCursor(0, 9);
            display.println("OK");
            display.setCursor(0, 17);
            display.println(WiFi.localIP());
            display.display();
            delay(2000); // to make time for the user to see
#endif
        } else {
#if defined(USE_DISPLAY)
            display.setCursor(0, 9);
            display.println("FAIL");
            display.display();
            delay(2000); // to make time for the user to see
#endif
        }
    }

    void Task() {
        // Automatic reconnect
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("WiFi lost, reconnecting...");
            WiFi.reconnect();
            wifiConnected = false;
        } else {
            wifiConnected = true;
        }

        // Portal trigger: only if WiFi is not connected or the user requests the portal
        if ((!wifiConnected || portalRequested)) {
            portalRequested = false; // reset flag
            Serial.println("Starting portal...");
            wifiManager.setConfigPortalTimeout(180); // timeout for portal
            wifiManager.startConfigPortal(); // blocking portal
            // after portal try check connection
            wifiConnected = (WiFi.status() == WL_CONNECTED);
        }
    }
}
