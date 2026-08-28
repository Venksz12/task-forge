#pragma once
#include "taskforge/metrics.hpp"
#include <cstdint>
#include <string>
namespace taskforge { class HttpServer{std::uint16_t port_;std::string token_;Metrics metrics_;public:HttpServer(std::uint16_t,std::string);int run();}; }
