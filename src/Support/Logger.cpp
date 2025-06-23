#include "Logger.h"

Logger GlobalLogger;

#define LOGGER_GET_TIME time(nullptr)

LogEntry::LogEntry() : timestamp(0),
      level(Loglevel::Info),
      errorCode(0),
      text(nullptr),
      isCode(true) {}

void LogEntry::Set(time_t time, Loglevel _level, uint32_t _errorCode) {
    timestamp = time;
    level = _level;
    errorCode = _errorCode;
    if (text != nullptr) { free(text); text = nullptr; }
    isCode = true;
}
void LogEntry::Set(time_t time, Loglevel _level, const __FlashStringHelper* _message) {
    timestamp = time;
    level = _level;
    message = _message;
    if (text != nullptr) { free(text); text = nullptr; }
    isCode = false;
}
void LogEntry::Set(time_t time, Loglevel _level, uint32_t _errorCode, const char* _text) {
    timestamp = time;
    level = _level;
    errorCode = _errorCode;
    if (text != nullptr) { free(text); text = nullptr; }
    if (_text) {
        text = strdup(_text);  // <-- heap-allocate and copy the string
    } else {
        text = nullptr;
    }
    isCode = true;
}
void LogEntry::Set(time_t time, Loglevel _level, const __FlashStringHelper* _message, const char* _text) {
    timestamp = time;
    level = _level;
    message = _message;
    if (text != nullptr) { free(text); text = nullptr; }
    if (_text) {
        text = strdup(_text);  // <-- heap-allocate and copy the string
    } else {
        text = nullptr;
    }
    isCode = false;
}
LogEntry::~LogEntry() {
    if (text) {
        free(text);
        text = nullptr;
    }
}

Logger::Logger() {

}

void Logger::Error(uint32_t code) {
    buffer[head].Set(LOGGER_GET_TIME, Loglevel::Error, code);
    advance();
}
void Logger::Error(const __FlashStringHelper* msg) {
    buffer[head].Set(LOGGER_GET_TIME, Loglevel::Error, msg);
    advance();
}
void Logger::Error(uint32_t code, const char* text) {
    buffer[head].Set(LOGGER_GET_TIME, Loglevel::Error, code, text);
    advance();
}
void Logger::Error(const __FlashStringHelper* msg, const char* text) {
    buffer[head].Set(LOGGER_GET_TIME, Loglevel::Error, msg, text);
    advance();
}

void Logger::Info(uint32_t code) {
    buffer[head].Set(LOGGER_GET_TIME, Loglevel::Info, code);
    advance();
}
void Logger::Info(const __FlashStringHelper* msg) {
    buffer[head].Set(LOGGER_GET_TIME, Loglevel::Info, msg);
    advance();
}
void Logger::Info(uint32_t code, const char* text) {
    buffer[head].Set(LOGGER_GET_TIME, Loglevel::Info, code, text);
    advance();
}
void Logger::Info(const __FlashStringHelper* msg, const char* text) {
    buffer[head].Set(LOGGER_GET_TIME, Loglevel::Info, msg, text);
    advance();
}

void Logger::Warn(uint32_t code) {
    buffer[head].Set(LOGGER_GET_TIME, Loglevel::Warn, code);
    advance();
}
void Logger::Warn(const __FlashStringHelper* msg) {
    buffer[head].Set(LOGGER_GET_TIME, Loglevel::Warn, msg);
    advance();
}
void Logger::Warn(uint32_t code, const char* text) {
    buffer[head].Set(LOGGER_GET_TIME, Loglevel::Warn, code, text);
    advance();
}
void Logger::Warn(const __FlashStringHelper* msg, const char* text) {
    buffer[head].Set(LOGGER_GET_TIME, Loglevel::Warn, msg, text);
    advance();
}


void Logger::printAllLogs(Stream &out) const {
    size_t start = wrapped ? head : 0;
    size_t count = wrapped ? LOG_BUFFER_SIZE : head;

    for (size_t i = 0; i < count; ++i) {
        size_t index = (start + i) % LOG_BUFFER_SIZE;
        const LogEntry& entry = buffer[index];

        out.print(F("["));
        struct tm* timeinfo;
        timeinfo = localtime(&entry.timestamp);
        out.print(asctime(timeinfo));
        //out.print(entry.timestamp);
        out.print(F("] "));

        switch (entry.level) {
            case Loglevel::Info: out.print(F("[INFO] ")); break;
            case Loglevel::Warn: out.print(F("[WARN] ")); break;
            case Loglevel::Error: out.print(F("[ERR] ")); break;
        }

        if (entry.isCode) {
            out.print(F("Error Code: 0x"));
            out.println(entry.errorCode, HEX);
        } else {
            out.println(entry.message);
        }
    }
}

void Logger::advance() {
    head = (head + 1) % LOG_BUFFER_SIZE;
    if (head == 0) wrapped = true;
}
