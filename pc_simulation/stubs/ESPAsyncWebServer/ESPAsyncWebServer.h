#pragma once

#include <functional>
#include <string>

#define HTTP_ANY 0

class AsyncWebServerRequest {
public:
    void send(int code, const char* contentType, const char* body);
    void send(int code, std::string* contentType, const char* body);
    std::string getParam(const std::string& name) const; // stub if needed
    std::string url() const; // stub if needed
};

using RequestHandlerFunction = std::function<void(AsyncWebServerRequest*)>;

class AsyncWebServer {
public:
    AsyncWebServer(int port);
    ~AsyncWebServer();
    void on(const std::string& uri, int method, RequestHandlerFunction handler);
    void begin();
};
