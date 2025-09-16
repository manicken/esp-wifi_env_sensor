
#define WIFI_MANAGER_WRAPPER_H_

#define USE_DISPLAY

#if defined(USE_DISPLAY)
// Display
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
//#include <Fonts/FreeMono9pt7b.h>
void WiFiManager_Handler(Adafruit_SSD1306& display);
#else
void WiFiManager_Handler();
#endif

