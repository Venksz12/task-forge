#include <gtest/gtest.h>
#include "taskforge/workflow.hpp"
#include "taskforge/scheduler.hpp"
#include "taskforge/retry.hpp"
#include "taskforge/thread_pool.hpp"
using namespace taskforge;
TEST(DAG,Order){Workflow w;w.nodes={{"A","",{}},{"B","",{}},{"C","",{}}};w.dependencies={{"A","C"},{"B","C"}};auto r=WorkflowEngine{}.topological_order(w);EXPECT_FALSE(r.cycle);EXPECT_EQ(r.order.size(),3u);}
TEST(DAG,Cycle){Workflow w;w.nodes={{"A","",{}},{"B","",{}}};w.dependencies={{"A","B"},{"B","A"}};EXPECT_TRUE(WorkflowEngine{}.topological_order(w).cycle);}
TEST(Retry,Exponential){RetryPolicy p{4,std::chrono::milliseconds(100),std::chrono::milliseconds(1000),{}};EXPECT_EQ(RetryCalculator::delay(1,p).count(),100);EXPECT_EQ(RetryCalculator::delay(2,p).count(),200);EXPECT_EQ(RetryCalculator::delay(3,p).count(),400);EXPECT_EQ(RetryCalculator::delay(4,p).count(),800);}
TEST(ThreadPool,Futures){ThreadPool p(4);auto f=p.submit([]{return 42;});EXPECT_EQ(f.get(),42);auto e=p.submit([]()->int{throw std::runtime_error("x");});EXPECT_THROW(e.get(),std::runtime_error);p.shutdown();}
TEST(Scheduler,AllClasses){FairScheduler s;for(int i=0;i<30;++i){Job j;j.id="h"+std::to_string(i);j.priority=Priority::High;s.enqueue(j);}for(int i=0;i<10;++i){Job j;j.id="n"+std::to_string(i);j.priority=Priority::Normal;s.enqueue(j);}for(int i=0;i<5;++i){Job j;j.id="l"+std::to_string(i);j.priority=Priority::Low;s.enqueue(j);}bool h=false,n=false,l=false;for(int i=0;i<20;++i){auto j=s.next();ASSERT_TRUE(j);h|=j->priority==Priority::High;n|=j->priority==Priority::Normal;l|=j->priority==Priority::Low;}EXPECT_TRUE(h&&n&&l);}
