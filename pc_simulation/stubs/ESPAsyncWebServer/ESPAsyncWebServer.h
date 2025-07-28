
#pragma once

#include <Arduino.h>
#include <functional>
#include <string>

#define HTTP_ANY 0

class AsyncWebServerRequest {
public:
    std::string _url;
    void send(int code, const char* contentType, const char* body);
    void send(int code, const char* contentType, String& body);
    
    std::string getParam(const std::string& name) const; // stub if needed
    const std::string& url() const; // stub if needed
};

using RequestHandlerFunction = std::function<void(AsyncWebServerRequest*)>;

class AsyncWebServer {
public:
    AsyncWebServer(int port);
    ~AsyncWebServer();
    void on(const std::string& uri, int method, RequestHandlerFunction handler);
    void onNotFound(RequestHandlerFunction handler);
    void begin();
};
