
#pragma once
#ifdef INPUT
#undef INPUT
#endif
#include <httplib.h>  // Add to your project
#include <functional>

#define HAL_JSON_REST_API_PORT 82

using HAL_JSON_REST_CB = std::function<std::string(const std::string&)>;

namespace HAL_JSON {
    class REST {
    private:
        static httplib::Server server;
        static HAL_JSON_REST_CB callback;
    public:
        static void setup(HAL_JSON_REST_CB cb);
    };
}