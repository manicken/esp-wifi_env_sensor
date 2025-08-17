
#pragma once

#define HAL_JSON_NOCOPY_NOMOVE(T) \
    T(const T&) = delete; \
    T& operator=(const T&) = delete; \
    T(T&&) = delete; T& operator=(T&&) = delete;

namespace HAL_JSON {
    namespace Rules {
        template<typename T>
        void DeleteAs(void* ptr) {
            delete static_cast<T*>(ptr);
        }
        template<typename T>
        void DeleteAsArray(void* ptr) {
            delete[] static_cast<T*>(ptr);
        }
        
        typedef void (*Deleter)(void* context);
    }
}