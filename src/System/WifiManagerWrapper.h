
#define WIFI_MANAGER_WRAPPER_H_

#define WIFI_MANAGER_AP_PASSWORD "haljson" // to be changed, TODO override this in a secrets.h file that is not included to github

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
bool Setup(Adafruit_SSD1306& display);
#else
bool Setup();
#endif

void Task();

}