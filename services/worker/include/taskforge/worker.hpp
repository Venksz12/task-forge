#pragma once
#include "taskforge/thread_pool.hpp"
#include <atomic>
#include <string>
namespace taskforge { class WorkerRuntime{std::string id_;ThreadPool pool_;std::atomic_bool stopping_{false};public:WorkerRuntime(std::string id,std::size_t n):id_(std::move(id)),pool_(n){}void run();void stop(){stopping_=true;}}; }
