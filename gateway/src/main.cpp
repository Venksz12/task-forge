#include "taskforge/http_server.hpp"
#include <cstdlib>
int main(){const auto*p=std::getenv("TASKFORGE_PORT");const auto*t=std::getenv("TASKFORGE_API_TOKEN");return taskforge::HttpServer(static_cast<std::uint16_t>(p?std::stoi(p):8080),t?t:"").run();}
