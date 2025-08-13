
#pragma once

namespace HAL_JSON {
    namespace Rule {
        template<typename T>
        void DeleteAs(void* ptr) {
            delete static_cast<T*>(ptr);
        }
        
        typedef void (*Deleter)(void* context);
    }
}