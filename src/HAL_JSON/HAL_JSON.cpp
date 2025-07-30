
#include "HAL_JSON.h"

namespace HAL_JSON {
    void begin() {
        REST::setup();
        Manager::setup();
    }

    void loop() {
        Manager::loop();
        // RULE loop call here when done
    }
}