
#pragma once

#define HAL_JSON_NOCOPY_NOMOVE(T) \
    T(const T&) = delete; \
    T& operator=(const T&) = delete; \
    T(T&&) = delete; T& operator=(T&&) = delete;

namespace HAL_JSON {
    namespace ScriptEngine {
        template<typename T>
        void DeleteAs(void* ptr) {
            delete static_cast<T*>(ptr);
        }
        template<typename T1, typename T2>
        void DeleteAs(void* ptr1, void* ptr2) {
            delete static_cast<T1*>(ptr1);
            delete static_cast<T2*>(ptr2);
        }
        template<typename T>
        void DeleteAsArray(void* ptr) {
            delete[] static_cast<T*>(ptr);
        }
        
        typedef void (*Deleter)(void* context);
        typedef void (*Deleter2x)(void* context1,void* context2);
    }
}