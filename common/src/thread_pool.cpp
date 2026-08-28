#include "taskforge/thread_pool.hpp"
#include <stdexcept>
namespace taskforge {
ThreadPool::ThreadPool(std::size_t n){if(n==0)throw std::invalid_argument("worker count must be positive");for(std::size_t i=0;i<n;++i)threads_.emplace_back([this](std::stop_token st){worker(st);});}
ThreadPool::~ThreadPool(){shutdown();}
void ThreadPool::worker(std::stop_token st){
 for(;;){std::function<void()> task;{std::unique_lock lock(mutex_);cv_.wait(lock,st,[this]{return stopping_||!tasks_.empty();});if(tasks_.empty()){if(stopping_||st.stop_requested())return;continue;}task=std::move(tasks_.front());tasks_.pop();}try{task();}catch(...){/* packaged_task captures task exceptions in its future */}}
}
void ThreadPool::shutdown(){ {std::scoped_lock lock(mutex_);if(stopping_)return;stopping_=true;}cv_.notify_all();for(auto& t:threads_)t.request_stop();cv_.notify_all();threads_.clear();}
std::size_t ThreadPool::queued()const{std::scoped_lock lock(mutex_);return tasks_.size();}
}
