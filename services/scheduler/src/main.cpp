#include "taskforge/scheduler.hpp"
#include "taskforge/infrastructure.hpp"
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>
using namespace taskforge;
int main(){const char* d=std::getenv("TASKFORGE_PG_DSN");const char* n=std::getenv("TASKFORGE_NATS_HOST");try{PgStore db({d?d:"host=postgres port=5432 dbname=taskforge user=taskforge password=change-me-in-dev"});NatsClient bus(n?n:"nats",4222);FairScheduler q;for(;;){for(auto&j:db.list_jobs(100)){if(j.status==JobStatus::Queued)q.enqueue(j);}while(auto j=q.next()){db.update_status(j->id,JobStatus::Running,"scheduler");bus.publish("taskforge.jobs",j->id+"|"+j->execution_id+"|"+j->payload);}std::this_thread::sleep_for(std::chrono::milliseconds(250));}}catch(const std::exception&e){std::cerr<<"{\"event\":\"scheduler_fatal\",\"error\":\""<<e.what()<<"\"}\n";return 1;}}
