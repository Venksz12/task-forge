#include "taskforge/scheduler.hpp"
#include <algorithm>
#include <stdexcept>
namespace taskforge {
FairScheduler::FairScheduler(Config c):config_(c){if(c.high_weight+c.normal_weight+c.low_weight==0)throw std::invalid_argument("all weights cannot be zero");}
FairScheduler::Queue& FairScheduler::queue(Priority p){return p==Priority::High?high_:(p==Priority::Normal?normal_:low_);}
const FairScheduler::Queue& FairScheduler::queue(Priority p)const{return p==Priority::High?high_:(p==Priority::Normal?normal_:low_);}
void FairScheduler::enqueue(Job j){std::scoped_lock lock(mutex_);queue(j.priority).jobs.push_back(std::move(j));}
bool FairScheduler::cancel(const std::string& id){std::scoped_lock lock(mutex_);cancelled_ids_.push_back(id);return true;}
std::optional<Job> FairScheduler::next(TimePoint now){
 std::scoped_lock lock(mutex_);
 Queue* q[]={&high_,&normal_,&low_};const std::uint32_t w[]={config_.high_weight,config_.normal_weight,config_.low_weight};
 for(int i=0;i<3;++i)q[i]->deficit+=w[i];
 auto cancelled=[this](const std::string& id){return std::find(cancelled_ids_.begin(),cancelled_ids_.end(),id)!=cancelled_ids_.end();};
 for(auto* x:q)while(!x->jobs.empty()&&cancelled(x->jobs.front().id))x->jobs.pop_front();
 Queue* chosen=nullptr;TimePoint oldest=now;
 for(auto* x:q)if(!x->jobs.empty()&&now-x->jobs.front().created_at>=config_.starvation_limit&&(!chosen||x->jobs.front().created_at<oldest)){chosen=x;oldest=x->jobs.front().created_at;}
 if(!chosen)for(std::size_t i=0;i<3;++i){auto idx=(cursor_+i)%3;if(!q[idx]->jobs.empty()&&q[idx]->deficit>0){chosen=q[idx];cursor_=(idx+1)%3;break;}}
 if(!chosen)return std::nullopt;
 --chosen->deficit;Job j=std::move(chosen->jobs.front());chosen->jobs.pop_front();return j;
}
std::size_t FairScheduler::depth(Priority p)const{std::scoped_lock lock(mutex_);return queue(p).jobs.size();}
std::size_t FairScheduler::total_depth()const{std::scoped_lock lock(mutex_);return high_.jobs.size()+normal_.jobs.size()+low_.jobs.size();}
}
