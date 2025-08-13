#pragma once

#include <Arduino.h>

namespace HAL_JSON {
    namespace Rule {
        template<typename T>
        class RPNStack {
        public:
            int size;
            T* items;
            int sp;

            RPNStack() : size(0), items(nullptr), sp(0) {}
            explicit RPNStack(int size) { Init(size); }

            ~RPNStack() { delete[] items; }

            void Init(int newSize) {
                delete[] items; // free old memory
                size = newSize;
                sp = 0;
                items = new T[size];
            }

            bool GetFinalResult(T& value) {
                if (sp != 1) return false;
                value = items[0];
                return true;
            }

        private:
            RPNStack(const RPNStack&) = delete;
            RPNStack& operator=(const RPNStack&) = delete;
            RPNStack(RPNStack&&) = delete;
            RPNStack& operator=(RPNStack&&) = delete;
        };
        
        
    }
}