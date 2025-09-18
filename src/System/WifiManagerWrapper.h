
#define WIFI_MANAGER_WRAPPER_H_

#define USE_DISPLAY

#if defined(USE_DISPLAY)
// Display
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
//#include <Fonts/FreeMono9pt7b.h>
#endif

namespace WiFiManagerWrapper {
    
#if defined(USE_DISPLAY)
void Setup(Adafruit_SSD1306& display);
#else
void Setup();
#endif

void Task();

}