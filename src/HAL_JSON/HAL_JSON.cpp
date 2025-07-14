
#include "HAL_JSON.h"

namespace HAL_JSON {
    void begin(WEBSERVER_TYPE &srv) {
        GPIO_manager::REST::setup(srv);
        REST::setup();
        Manager::setup();
    }

    void loop() {
        Manager::loop();
        // RULE loop call here when done
    }
}