
#pragma once


#include <Arduino.h>

#include <stdlib.h>

#include <ArduinoJson.h>

#include "../../Support/Logger.h"
#include "../../Support/ConvertHelper.h"

#include "../HAL_JSON_Device_GlobalDefines.h"

#include "../HAL_JSON_Manager.h"

#include "../HAL_JSON_UID_Path.h"
#include "../HAL_JSON_Value.h"
#include "../HAL_JSON_Operations.h"

#include <vector> // have this for easy prototyping
#include <variant>

namespace HAL_JSON {
    namespace Rule {
        /** 
         * this will be a pure static class 
         * i.e. only static members
         */
        class Engine {
        private:
            

        public:

        };
    }
}