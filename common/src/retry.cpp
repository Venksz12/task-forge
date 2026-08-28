#include "taskforge/retry.hpp"
namespace taskforge {
std::chrono::milliseconds RetryCalculator::delay(std::uint32_t attempt,const RetryPolicy& p,std::chrono::milliseconds jitter){
 if(attempt==0)return {};
 std::uint64_t value=static_cast<std::uint64_t>(p.base_delay.count()),cap=static_cast<std::uint64_t>(p.max_delay.count());
 for(std::uint32_t i=1;i<attempt&&value<cap;++i)value=std::min<std::uint64_t>(cap,value*2);
 value=std::min<std::uint64_t>(cap,value+static_cast<std::uint64_t>(std::max<std::int64_t>(0,jitter.count())));
 return std::chrono::milliseconds(value);
}
}
