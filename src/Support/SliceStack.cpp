#include "SliceStack.h"

void SliceStackReportError(const char* msg, const char* param) {
#if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
            std::cout << "SliceStackError: " << msg << " " << ((param!=nullptr)?param:"") << std::endl;
#else
            GlobalLogger.Error(F("SliceStack:"), msg);
#endif
        }