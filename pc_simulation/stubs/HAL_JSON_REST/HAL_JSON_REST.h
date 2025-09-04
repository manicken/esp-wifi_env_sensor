
#pragma once
#ifdef _WIN32
#ifdef INPUT
#undef INPUT
#endif
#include <httplib.h>  // Add to your project
#endif
#include <functional>
#include <string>

#define HAL_JSON_REST_API_PORT 82

using HAL_JSON_REST_CB = std::function<std::string(const std::string&)>;

namespace HAL_JSON {
    class REST {
    private:
#ifdef _WIN32
        static httplib::Server server;
#endif
        static HAL_JSON_REST_CB callback;
    public:
        static void setup(HAL_JSON_REST_CB cb);
    };
}