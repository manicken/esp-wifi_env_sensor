
#include <Arduino.h>
#include "Logger.h"

void SliceStackReportError(const char* msg, const char* param);

        template<typename T, const char* TypeName>
        struct SliceStack {
            static constexpr const char* Name = TypeName;
            T* data;
            int minIndex;     // beginning of current slice
            int currIndex;    // next free slot
            int stackSize;
            int maxUsage;

            inline SliceStack() = delete;

            inline SliceStack(int _size) {
                data = new T[_size];
                stackSize = _size;
                minIndex = 0;
                currIndex = 0;
                maxUsage = 0;
            }
            inline ~SliceStack() {
                delete[] data;
            }
            // push
            inline void push(const T& v) {
                if (currIndex >= stackSize) {
                    SliceStackReportError("push - stack overflow:",Name);
                    return;
                }
                data[currIndex++] = v;
                if (currIndex > maxUsage)
                    maxUsage = currIndex;
            }

            // pop
            inline T top_n_pop() {
                if (currIndex == minIndex) {
                    SliceStackReportError("top_n_pop - stack underflow:",Name);
                    return nullptr;
                }
                return data[--currIndex];
            }

            inline void pop() {
                if (currIndex == minIndex) {
                    SliceStackReportError("pop - stack underflow:",Name);
                    return;
                }
                currIndex--;
            }
            inline void collapseTop() {
                if (currIndex - minIndex < 2) {
                    SliceStackReportError("pop - stack underflow:",Name);
                    return;
                }
                data[currIndex-2] = data[currIndex-1];
                currIndex--;
            }

            // top
            inline T& top() {
                if (currIndex == minIndex) {
                    SliceStackReportError("top - stack underflow:",Name);
                    static T dummy{}; // reference to dummy
                    return dummy;
                }
                return data[currIndex - 1];
            }

            // size of *this slice*
            inline int size() const {
                return currIndex - minIndex;
            }

            // check empty
            inline bool empty() const {
                return currIndex == minIndex;
            }

            // check not empty
            inline bool notEmpty() const {
                return currIndex != minIndex;
            }

            inline void ClearCurrSlice() {
                currIndex = minIndex;
            }

            // restore slice (like leaving it)
            inline void Restore(int prevMin, int prevCurr) {
                minIndex = prevMin;
                currIndex = prevCurr;
            }

            // start a nested slice (local context)
            inline void BeginSlice(int& _currMinIndex, int& _currIndex) {
                _currIndex = currIndex;
                _currMinIndex = minIndex;
                minIndex = currIndex;
            }

            inline T& operator[](int idx) const {
                if (idx < 0 || idx >= currIndex - minIndex) {
                    SliceStackReportError("stack out of bounds",Name);
                    static T dummy{}; // reference to dummy
                    return dummy;
                }
                return data[minIndex + idx];
            }
        };