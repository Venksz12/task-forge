#pragma once
#include <algorithm>
#include <chrono>
#include <cstdint>
namespace taskforge {
struct RetryPolicy{std::uint32_t max_attempts{4};std::chrono::milliseconds base_delay{250};std::chrono::milliseconds max_delay{30000};std::chrono::milliseconds jitter_max{100};};
class RetryCalculator{
public:
 static std::chrono::milliseconds delay(std::uint32_t attempt,const RetryPolicy&,std::chrono::milliseconds jitter={});
 static bool retryable(std::uint32_t attempt,const RetryPolicy& p){return attempt<p.max_attempts;}
};
}
