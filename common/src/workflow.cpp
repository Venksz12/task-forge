#include "taskforge/workflow.hpp"
#include <queue>
namespace taskforge {
WorkflowEngine::Resolution WorkflowEngine::topological_order(const Workflow& w)const{
 std::unordered_map<std::string,int> in;std::unordered_map<std::string,std::vector<std::string>> out;
 for(auto& n:w.nodes)in[n.id]=0;
 for(auto& [a,b]:w.dependencies){++in[b];out[a].push_back(b);}
 std::queue<std::string> q;for(auto& [id,d]:in)if(d==0)q.push(id);
 Resolution r;while(!q.empty()){auto id=q.front();q.pop();r.order.push_back(id);for(auto& c:out[id])if(--in[c]==0)q.push(c);}
 r.cycle=r.order.size()!=w.nodes.size();return r;
}
std::vector<std::string> WorkflowEngine::ready_nodes(const Workflow& w,const std::unordered_map<std::string,NodeStatus>& s)const{
 std::unordered_map<std::string,std::vector<std::string>> deps;for(auto& [a,b]:w.dependencies)deps[b].push_back(a);
 std::vector<std::string> out;for(auto& n:w.nodes){if(s.contains(n.id)&&s.at(n.id)!=NodeStatus::Pending)continue;bool ready=true;for(auto& d:deps[n.id])if(!s.contains(d)||s.at(d)!=NodeStatus::Succeeded){ready=false;break;}if(ready)out.push_back(n.id);}return out;
}
bool WorkflowEngine::should_block(const std::string& id,const Workflow& w,const std::unordered_map<std::string,NodeStatus>& s)const{
 for(auto& [a,b]:w.dependencies)if(b==id&&s.contains(a)&&(s.at(a)==NodeStatus::Failed||s.at(a)==NodeStatus::Blocked))return true;return false;
}
}
