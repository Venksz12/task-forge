#pragma once
#include <atomic>
#include <cstdint>
#include <string>
namespace taskforge {
class Metrics{
 std::atomic<std::uint64_t> submitted_{0},completed_{0},failed_{0},retried_{0},reassigned_{0},queue_{0},workers_{0};
public:
 void submitted(){++submitted_;} void completed(){++completed_;} void failed(){++failed_;} void retried(){++retried_;} void reassigned(){++reassigned_;}
 void queue(std::uint64_t v){queue_.store(v);} void workers(std::uint64_t v){workers_.store(v);}
 std::string prometheus()const;
};
}
