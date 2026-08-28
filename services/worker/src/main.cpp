#include "taskforge/infrastructure.hpp"
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>
using namespace taskforge;
int main(){const char* id=std::getenv("TASKFORGE_WORKER_ID");const char* d=std::getenv("TASKFORGE_PG_DSN");try{PgStore db({d?d:"host=postgres port=5432 dbname=taskforge user=taskforge password=change-me-in-dev"});RedisClient redis(std::getenv("TASKFORGE_REDIS_HOST")?std::getenv("TASKFORGE_REDIS_HOST"):"redis",6379);NatsClient bus(std::getenv("TASKFORGE_NATS_HOST")?std::getenv("TASKFORGE_NATS_HOST"):"nats",4222);std::string wid=id?id:"worker-local";for(;;){redis.setex("taskforge:worker:"+wid,15,"ready");auto m=bus.next("taskforge.jobs");if(!m)continue;auto sep=m->second.find('|');auto sep2=m->second.find('|',sep+1);auto jid=m->second.substr(0,sep);auto eid=m->second.substr(sep+1,sep2-sep-1);auto j=db.get_job(jid);if(!j||j->execution_id!=eid)continue;if(j->status==JobStatus::Succeeded||j->status==JobStatus::Cancelled)continue;db.record_attempt(*j,wid,"RUNNING");std::this_thread::sleep_for(std::chrono::milliseconds(10));db.record_attempt(*j,wid,"SUCCEEDED");db.update_status(jid,JobStatus::Succeeded,wid);}}
catch(const std::exception&e){std::cerr<<"{\"event\":\"worker_fatal\",\"error\":\""<<e.what()<<"\"}\n";return 1;}}
