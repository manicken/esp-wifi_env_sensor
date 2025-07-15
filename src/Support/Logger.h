#pragma once

#ifndef _WIN32
#include <Arduino.h>
#include <ArduinoJson.h>
#else
#include <cstring>
#define strdup _strdup
#define __FlashStringHelper char 
#include <string>    // std::string for Windows
#include <cstdint>
using String = std::string;
#define F(x) x
#include "Stream_WIN.h"
static Stream Serial(std::cout);  // Simulate Arduino Serial
#endif
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
      bool isNew = false;
      LogEntry();
      void Set(time_t time, Loglevel level, uint32_t errorCode);
      void Set(time_t time, Loglevel level, const __FlashStringHelper* message);
      void Set(time_t time, Loglevel level, uint32_t errorCode, const char* text);
      void Set(time_t time, Loglevel level, const __FlashStringHelper* message, const char* text);

      ~LogEntry();

      // Delete copy constructor and copy assignment
      LogEntry(const LogEntry&) = delete;
      LogEntry& operator=(const LogEntry&) = delete;

      String MessageToString() const;
  };

class Logger {

    
  public:
    Logger();
    void Error(uint32_t code);
    void Error(const __FlashStringHelper* msg);
    void Error(uint32_t code, const char* text);
    void Error(const __FlashStringHelper* msg, const char* text);
#ifndef _WIN32
    void Error(const __FlashStringHelper* msg, const JsonVariant& jsonObj);
#endif
    void Info(uint32_t code);
    void Info(const __FlashStringHelper* msg);
    void Info(uint32_t code, const char* text);
    void Info(const __FlashStringHelper* msg, const char* text);
#ifndef _WIN32
    void Info(const __FlashStringHelper* msg, const JsonVariant& jsonObj);
#endif
    void Warn(uint32_t code);
    void Warn(const __FlashStringHelper* msg);
    void Warn(uint32_t code, const char* text);
    void Warn(const __FlashStringHelper* msg, const char* text);
#ifndef _WIN32
    void Warn(const __FlashStringHelper* msg, const JsonVariant& jsonObj);
    void printAllLogs(Stream &out = Serial, bool onlyPrintNew = false);
#else
    void printAllLogs(Stream &out = Serial, bool onlyPrintNew = false);
#endif
    
    const LogEntry& getLastEntry() const;

  private:
    static constexpr size_t LOG_BUFFER_SIZE = 128;
    LogEntry buffer[LOG_BUFFER_SIZE];
    size_t head = 0;
    bool wrapped = false;
    void addAndAdvance(LogEntry&& entry);
    void advance();
};
/*
class PrintStreamAdapter : public Stream {
public:
  PrintStreamAdapter(Print &p) : _print(p) {}

  size_t write(uint8_t b) override { return _print.write(b); }
  size_t write(const uint8_t *buffer, size_t size) override { return _print.write(buffer, size); }

  int available() override { return 0; }
  int read() override { return -1; }
  int peek() override { return -1; }

private:
  Print &_print;
};
*/

// Declare a global instance
extern Logger GlobalLogger;

