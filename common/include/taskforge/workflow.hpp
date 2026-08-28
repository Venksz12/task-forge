#pragma once
#include "taskforge/models.hpp"
#include <unordered_map>
#include <vector>
namespace taskforge {
class WorkflowEngine {
public:
 struct Resolution{std::vector<std::string> order;bool cycle{false};};
 Resolution topological_order(const Workflow&)const;
 std::vector<std::string> ready_nodes(const Workflow&,const std::unordered_map<std::string,NodeStatus>&)const;
 bool should_block(const std::string&,const Workflow&,const std::unordered_map<std::string,NodeStatus>&)const;
};
}
