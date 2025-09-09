
#include "HAL_JSON.h"

namespace HAL_JSON {
    void begin() {
        HAL_JSON::REST::setupRest();
        if (Manager::setupMgr() == false)
            return;
        ScriptEngine::ValidateAndLoadAllActiveScripts(); 
    }


    long lastmillis = 0;
    void loop() {
        if (Manager::reloadQueued) {
            Manager::reloadQueued = false;
            //Manager::setup(); // this only reloads the cfg and run begin on all loaded devices
            //ScriptEngine::ValidateAndLoadAllActiveScripts();
        }

        long currmillis = millis();
        if (currmillis-lastmillis > 100) {
            lastmillis = currmillis;
            Manager::loop();
            //if (ScriptEngine::ScriptsBlock::running)
                ScriptEngine::Exec(); // runs the scriptengine
        }
    }
}