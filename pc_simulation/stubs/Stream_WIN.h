#pragma once

#include <Arduino.h>

#include <iostream>
#include <iomanip>   // for std::hex, etc.
#include <string>

#ifndef DEC
#define DEC 10
#endif
#ifndef HEX
#define HEX 16
#endif

class Stream {
    std::ostream& out;

public:
    Stream(std::ostream& stream = std::cout) : out(stream) {}
    bool available() { return false; }
    uint8_t read() { return 0; }
    void print(const std::string& s) { out << s; }
    void print(const char* s)        { out << s; }
    void print(char c)               { out << c; }
    void print(int n, int format = DEC) {
        if (format == HEX)
            out << "0x" << std::hex << n << std::dec;
        else
            out << n;
    }
    void println() { out << std::endl; }
    void println(const std::string& s) { out << s << std::endl; }
    void println(const char* s)        { out << s << std::endl; }
};



