#ifdef _WIN32
#ifdef INPUT
#undef INPUT
#endif
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#endif

#include "HAL_JSON_REST.h"

//using namespace httplib;
#ifdef _WIN32
static httplib::Server server;
#endif

namespace HAL_JSON {

    // Define the static server
    HAL_JSON_REST_CB REST::callback;  // define the static member here
#ifdef _WIN32
    httplib::Server REST::server;
#endif
    void REST::setup(HAL_JSON_REST_CB cb) {
        
        REST::callback = cb;
#ifdef _WIN32
        // Example route
        server.set_error_handler([](const httplib::Request& req, httplib::Response& res) {
            //if (res.status != 404) return; // only intercept 404 errors
            std::cout << "req.path:" << req.path << "\n";
            res.set_content(REST::callback(req.path), "application/json");
        });
        WSADATA wsaData;
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (result != 0) {
            std::cerr << "WSAStartup failed: " << result << std::endl;
            return; // exit thread early if failed
        }
        std::cout << "Winsock initialized.\n";

        int port = server.bind_to_port("0.0.0.0", HAL_JSON_REST_API_PORT);
        if (port > 0) {
            std::cout << "*** REST API server started on port " << HAL_JSON_REST_API_PORT << "\n";
            std::cout << "*** can be accessed by http://localhost:82 \n";
            std::thread serverThread([]() {
                server.listen_after_bind();
                WSACleanup();
                std::cout << "Winsock cleaned up.\n";
            });
            serverThread.detach();
        } else {
            WSACleanup();
            std::cerr << "Failed to bind REST API server.\n";
        }
#endif
    }
}
