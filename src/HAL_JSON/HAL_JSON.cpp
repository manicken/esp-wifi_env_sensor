
#include "HAL_JSON.h"

namespace HAL_JSON {
    void begin() {
        REST::setup();
        Manager::setup();
        Rules::Parser::ReadAndParseRuleSetFile("script.txt");
    }

    void loop() {
        Manager::loop();
        // RULE loop call here when done
    }
}