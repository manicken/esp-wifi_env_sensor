
#include "WiFiManager.h"
#include "WifiManagerWrapper.h"
#include "../Support/NTP.h"

#if defined(USE_DISPLAY)
// Display
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#endif

namespace WiFiManagerWrapper {

    WiFiManager wifiManager;
    bool portalRunning = false;

#if defined(USE_DISPLAY)
    bool Setup(Adafruit_SSD1306& display) {
#else
    bool Setup() {
#endif

        Serial.println(F("trying to connect to saved wifi"));
#if defined(USE_DISPLAY)
        display.setCursor(0, 0);
        display.println(F("WiFi connecting..."));
        display.display();
#endif

        // Try connect to saved WiFi, portal starts if it fails
        wifiManager.setConfigPortalBlocking(true); // default
        wifiManager.setConfigPortalTimeout(180); // 3 min to run the portal otherwise it will continue
        // autoConnect is using ESP.getChipId() internally
        wifiManager.autoConnect(wifiManager.getDefaultAPName().c_str(), WIFI_MANAGER_AP_PASSWORD);
        bool wifiConnected = (WiFi.status() == WL_CONNECTED);

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
        return wifiConnected;
    }

    void startPortalNonBlocking() {
        wifiManager.setConfigPortalBlocking(false);
        wifiManager.startConfigPortal();
        portalRunning = true;
    }

    void Task() {
        // Automatic reconnect
        static unsigned long lastReconnectAttempt = 0;

        if (WiFi.status() != WL_CONNECTED) {
            if (millis() - lastReconnectAttempt > 10000) { // every 10 seconds
                // request reconnect (non-blocking, connection happens in background)
                if (WiFi.reconnect())
                    Serial.println("WiFi lost, trying reconnect...");
                else
                    Serial.println("WiFi lost, reconnect could not be started...");
                lastReconnectAttempt = millis();
                if (portalRunning == false)
                    startPortalNonBlocking();
            }
        } else {
            if (portalRunning) {
                portalRunning = false;
                NTP::NTPConnect();
            }
        }
        if (portalRunning)
            wifiManager.process();
    }
}
