#pragma once
#include <condition_variable>
#include <cstddef>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <stop_token>
#include <thread>
#include <type_traits>
#include <vector>
namespace taskforge {
class ThreadPool {
public:
 explicit ThreadPool(std::size_t workers);
 ~ThreadPool();
 ThreadPool(const ThreadPool&) = delete;
 ThreadPool& operator=(const ThreadPool&) = delete;
 template<class F,class...A>
 auto submit(F&& f,A&&...a)->std::future<std::invoke_result_t<F,A...>> {
   using R=std::invoke_result_t<F,A...>;
   auto task=std::make_shared<std::packaged_task<R()>>(std::bind(std::forward<F>(f),std::forward<A>(a)...));
   auto result=task->get_future();
   { std::scoped_lock lock(mutex_); if(stopping_) throw std::runtime_error("thread pool is stopping"); tasks_.emplace([task]{(*task)();}); }
   cv_.notify_one();
   return result;
 }
 void shutdown();
 std::size_t queued() const;
private:
 void worker(std::stop_token);
 mutable std::mutex mutex_;
 std::condition_variable_any cv_;
 std::queue<std::function<void()>> tasks_;
 std::vector<std::jthread> threads_;
 bool stopping_{false};
};
}
