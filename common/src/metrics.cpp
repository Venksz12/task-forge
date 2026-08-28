#include "taskforge/metrics.hpp"
#include <sstream>
namespace taskforge {
std::string Metrics::prometheus()const{std::ostringstream o;o<<"taskforge_jobs_submitted_total "<<submitted_.load()<<"\n";o<<"taskforge_jobs_completed_total "<<completed_.load()<<"\n";o<<"taskforge_jobs_failed_total "<<failed_.load()<<"\n";o<<"taskforge_jobs_retried_total "<<retried_.load()<<"\n";o<<"taskforge_jobs_reassigned_total "<<reassigned_.load()<<"\n";o<<"taskforge_queue_depth "<<queue_.load()<<"\n";o<<"taskforge_worker_count "<<workers_.load()<<"\n";return o.str();}
}
