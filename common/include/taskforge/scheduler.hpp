#pragma once
#include "taskforge/models.hpp"
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <mutex>
#include <optional>
namespace taskforge {
class FairScheduler {
public:
 struct Config { std::uint32_t high_weight{70},normal_weight{20},low_weight{10}; std::chrono::seconds starvation_limit{30}; };
 explicit FairScheduler(Config config={});
 void enqueue(Job job);
 bool cancel(const std::string& id);
 std::optional<Job> next(TimePoint now=Clock::now());
 std::size_t depth(Priority)const;
 std::size_t total_depth()const;
private:
 struct Queue{std::deque<Job> jobs;std::int64_t deficit{0};};
 Queue& queue(Priority); const Queue& queue(Priority)const;
 Config config_; mutable std::mutex mutex_; std::deque<std::string> cancelled_ids_;
 Queue high_,normal_,low_; std::size_t cursor_{0};
};
}
