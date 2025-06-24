#pragma once

#include <Arduino.h>
#include <time.h>

enum class Loglevel : uint8_t {
    Info = 0,
    Warn = 1,
    Error = 2
};

struct LogEntry {
      time_t timestamp;
      Loglevel level;
      union {
          uint32_t errorCode;
          const __FlashStringHelper* message;
      };
      char* text;
      bool isCode;
      LogEntry();
      void Set(time_t time, Loglevel level, uint32_t errorCode);
      void Set(time_t time, Loglevel level, const __FlashStringHelper* message);
      void Set(time_t time, Loglevel level, uint32_t errorCode, const char* text);
      void Set(time_t time, Loglevel level, const __FlashStringHelper* message, const char* text);

      ~LogEntry();

      // Delete copy constructor and copy assignment
      LogEntry(const LogEntry&) = delete;
      LogEntry& operator=(const LogEntry&) = delete;
  };

class Logger {

    
  public:
    Logger();
    void Error(uint32_t code);
    void Error(const __FlashStringHelper* msg);
    void Error(uint32_t code, const char* text);
    void Error(const __FlashStringHelper* msg, const char* text);
    void Info(uint32_t code);
    void Info(const __FlashStringHelper* msg);
    void Info(uint32_t code, const char* text);
    void Info(const __FlashStringHelper* msg, const char* text);
    void Warn(uint32_t code);
    void Warn(const __FlashStringHelper* msg);
    void Warn(uint32_t code, const char* text);
    void Warn(const __FlashStringHelper* msg, const char* text);
    void printAllLogs(Stream &out = Serial) const;

  private:
    static constexpr size_t LOG_BUFFER_SIZE = 128;
    LogEntry buffer[LOG_BUFFER_SIZE];
    size_t head = 0;
    bool wrapped = false;
    void addAndAdvance(LogEntry&& entry);
    void advance();
};

// Declare a global instance
extern Logger GlobalLogger;

