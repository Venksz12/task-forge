#include "taskforge/models.hpp"
namespace taskforge {
std::string to_string(Priority v){switch(v){case Priority::High:return "HIGH";case Priority::Normal:return "NORMAL";case Priority::Low:return "LOW";}return "UNKNOWN";}
std::string to_string(JobStatus v){switch(v){case JobStatus::Submitted:return "SUBMITTED";case JobStatus::Queued:return "QUEUED";case JobStatus::Running:return "RUNNING";case JobStatus::Succeeded:return "SUCCEEDED";case JobStatus::Failed:return "FAILED";case JobStatus::Retrying:return "RETRYING";case JobStatus::DeadLetter:return "DEAD_LETTER";case JobStatus::Cancelled:return "CANCELLED";}return "UNKNOWN";}
std::string to_string(WorkerStatus v){switch(v){case WorkerStatus::Registering:return "REGISTERING";case WorkerStatus::Ready:return "READY";case WorkerStatus::Busy:return "BUSY";case WorkerStatus::Draining:return "DRAINING";case WorkerStatus::Unavailable:return "UNAVAILABLE";}return "UNKNOWN";}
std::string to_string(NodeStatus v){switch(v){case NodeStatus::Pending:return "PENDING";case NodeStatus::Ready:return "READY";case NodeStatus::Running:return "RUNNING";case NodeStatus::Succeeded:return "SUCCEEDED";case NodeStatus::Failed:return "FAILED";case NodeStatus::Blocked:return "BLOCKED";}return "UNKNOWN";}
}
