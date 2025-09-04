
#include "HAL_JSON.h"

namespace HAL_JSON {
    void begin() {
        REST::setup();
        Manager::setup();
        Rules::ScriptsBlock::ValidateAndLoadAllActiveScripts(); 
    }

    void loop() {
        Manager::loop();

        Rules::ScriptsBlock::Exec();
    }
}