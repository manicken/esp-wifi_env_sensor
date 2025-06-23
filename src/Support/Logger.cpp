#include "Logger.h"

Logger GlobalLogger;

void Logger::Log(uint32_t code, Loglevel level = Loglevel::Info) {
    buffer[head] = {millis(), level, .errorCode = code, true};
    advance();
}

void Logger::Log(const __FlashStringHelper* msg, Loglevel level = Loglevel::Info) {
    buffer[head] = {millis(), level, .message = msg, false};
    advance();
}

void Logger::Error(uint32_t code) {
    buffer[head] = {millis(), Loglevel::Error, .errorCode = code, true};
    advance();
}
void Logger::Error(const __FlashStringHelper* msg) {
    buffer[head] = {millis(), Loglevel::Error, .message = msg, false};
    advance();
}
void Logger::Info(uint32_t code) {
    buffer[head] = {millis(), Loglevel::Info, .errorCode = code, true};
    advance();
}
void Logger::Info(const __FlashStringHelper* msg) {
    buffer[head] = {millis(), Loglevel::Info, .message = msg, false};
    advance();
}
void Logger::Warn(uint32_t code) {
    buffer[head] = {millis(), Loglevel::Warn, .errorCode = code, true};
    advance();
}
void Logger::Warn(const __FlashStringHelper* msg) {
    buffer[head] = {millis(), Loglevel::Warn, .message = msg, false};
    advance();
}

void Logger::printAllLogs(Stream &out) const {
    size_t start = wrapped ? head : 0;
    size_t count = wrapped ? LOG_BUFFER_SIZE : head;

    for (size_t i = 0; i < count; ++i) {
        size_t index = (start + i) % LOG_BUFFER_SIZE;
        const LogEntry& entry = buffer[index];

        out.print(F("["));
        out.print(entry.timestamp);
        out.print(F("] "));

        switch (entry.level) {
            case Loglevel::Info: out.print(F("[INFO] ")); break;
            case Loglevel::Warn: out.print(F("[WARN] ")); break;
            case Loglevel::Err: out.print(F("[ERR] ")); break;
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
