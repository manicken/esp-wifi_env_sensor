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
    server.Get(uri.c_str(), [handler](const httplib::Request&, httplib::Response& res) {
        AsyncWebServerRequest req;
        handler(&req); // Simplify or map actual data as needed
        res.set_content("Stub response", "text/plain");
    });
}

void AsyncWebServer::begin() {
    printf("Web server running on http://localhost:%d\n", server_port);
    server.listen("0.0.0.0", server_port);
}

void AsyncWebServerRequest::send(int code, const char* contentType, const char* body) {
    printf("Sending [%d] %s\n%s\n", code, contentType, body);
}
