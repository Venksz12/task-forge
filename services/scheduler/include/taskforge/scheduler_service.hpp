#pragma once
#include "taskforge/scheduler.hpp"
namespace taskforge { class SchedulerService{FairScheduler scheduler_;public:FairScheduler& scheduler(){return scheduler_;}}; }
