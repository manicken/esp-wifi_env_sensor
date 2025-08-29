#include "SliceStack.h"

void SliceStackReportError(const char* msg, const char* param) {
#ifdef _WIN32
            std::cout << "SliceStackError: " << msg << " " << ((param!=nullptr)?param:"") << std::endl;
#else
            GlobalLogger.Error(F("SliceStack:"), msg);
#endif
        }