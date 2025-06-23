#pragma once

#include <Arduino.h>
#include <time.h>

enum class Loglevel : uint8_t {
    Info = 0,
    Warn = 1,
    Error = 2
};

class Logger {

    struct LogEntry {
      time_t timestamp;
      Loglevel level;
      union {
          uint32_t errorCode;
          const __FlashStringHelper* message;
      };
      bool isCode;
  };
  public:
    void Log(uint32_t code, Loglevel level = Loglevel::Info);
    void Log(const __FlashStringHelper* msg, Loglevel level = Loglevel::Info);
    void Error(uint32_t code);
    void Error(const __FlashStringHelper* msg);
    void Info(uint32_t code);
    void Info(const __FlashStringHelper* msg);
    void Warn(uint32_t code);
    void Warn(const __FlashStringHelper* msg);
    void printAllLogs(Stream &out = Serial) const;

  private:
    static constexpr size_t LOG_BUFFER_SIZE = 128;
    LogEntry buffer[LOG_BUFFER_SIZE];
    size_t head = 0;
    bool wrapped = false;

    void advance();
};

// Declare a global instance
extern Logger GlobalLogger;

