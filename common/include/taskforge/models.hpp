#pragma once
#include <chrono>
#include <cstdint>
#include <string>
#include <vector>
namespace taskforge {
using Clock = std::chrono::steady_clock;
using TimePoint = Clock::time_point;
enum class Priority { High, Normal, Low };
enum class JobStatus { Submitted, Queued, Running, Succeeded, Failed, Retrying, DeadLetter, Cancelled };
enum class WorkerStatus { Registering, Ready, Busy, Draining, Unavailable };
enum class NodeStatus { Pending, Ready, Running, Succeeded, Failed, Blocked };
std::string to_string(Priority);
std::string to_string(JobStatus);
std::string to_string(WorkerStatus);
std::string to_string(NodeStatus);
struct Job {
  std::string id;
  std::string execution_id;
  std::string name;
  Priority priority{Priority::Normal};
  JobStatus status{JobStatus::Submitted};
  std::string payload;
  std::chrono::milliseconds timeout{30000};
  std::uint32_t attempt{0};
  std::uint32_t max_attempts{4};
  std::string worker_id;
  TimePoint created_at{Clock::now()};
};
struct Worker {
  std::string id;
  std::string hostname;
  WorkerStatus status{WorkerStatus::Registering};
  std::uint32_t capacity{1};
  std::uint32_t active_jobs{0};
  TimePoint registered_at{Clock::now()};
  TimePoint last_heartbeat{Clock::now()};
  std::string version;
  bool healthy{false};
};
struct WorkflowNode { std::string id; std::string job_id; NodeStatus status{NodeStatus::Pending}; };
struct Workflow {
  std::string id;
  std::vector<WorkflowNode> nodes;
  std::vector<std::pair<std::string,std::string>> dependencies;
};
}
