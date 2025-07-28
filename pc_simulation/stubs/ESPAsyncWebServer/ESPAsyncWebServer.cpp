#define WIN32_LEAN_AND_MEAN  // Always do this before windows.h
#include <winsock2.h>        // Include this BEFORE windows.h
#include <ws2tcpip.h>        // Optional: for additional socket APIs
#include <windows.h>

#include "ESPAsyncWebServer.h"



#include <httplib.h>  // Add to your project

static httplib::Server server;
static int server_port = 80;

AsyncWebServer::AsyncWebServer(int port) {
    server_port = port;

    WSADATA wsaData;
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (result != 0) {
            std::cerr << "WSAStartup failed: " << result << std::endl;
            // Handle error appropriately
        } else {
            std::cout << "Winsock initialized.\n";
        }
}
AsyncWebServer::~AsyncWebServer() {
    WSACleanup();
        std::cout << "Winsock cleaned up.\n";
}

void AsyncWebServer::on(const std::string& uri, int method, RequestHandlerFunction handler) {
    server.Get(uri.c_str(), [handler](const httplib::Request& _req, httplib::Response& res) {
        
        AsyncWebServerRequest req;
        handler(&req); // Simplify or map actual data as needed
        res.set_content("Stub response", "text/plain");
    });
}
void AsyncWebServer::onNotFound(RequestHandlerFunction handler) {
    server.set_error_handler([handler](const httplib::Request& req, httplib::Response& res) {
        if (res.status != 404) return; // only intercept 404 errors

        // Wrap the request into your own AsyncWebServerRequest stub
        AsyncWebServerRequest request;
        request._url = req.path; // Or populate more if needed

        // Call the user-defined handler
        handler(&request);

        // Provide a default response if the handler didn’t already
        if (!res.body.length()) {
            res.set_content("Not Found", "text/plain");
        }
    });
}



void AsyncWebServer::begin() {
    printf("Web server running on http://localhost:%d\n", server_port);
    std::thread serverThread([this]() {
        server.listen("0.0.0.0", server_port);
    });
    serverThread.detach();
}

void AsyncWebServerRequest::send(int code, const char* contentType, const char* body) {
    printf("Sending [%d] %s\n%s\n", code, contentType, body);
}
void AsyncWebServerRequest::send(int code, const char* contentType, String& body) {
    printf("Sending [%d] %s\n%s\n", code, contentType, body);
}
const std::string& AsyncWebServerRequest::url() const {
    return _url;
}
