#include "taskforge/http_server.hpp"
#include "taskforge/auth.hpp"
#include "taskforge/infrastructure.hpp"
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/http.hpp>
#include <atomic>
#include <cstdlib>
#include <random>
#include <sstream>
namespace net=boost::asio; namespace beast=boost::beast; namespace http=beast::http; using tcp=net::ip::tcp;
namespace taskforge {
static std::string uuid(){static thread_local std::mt19937_64 g{std::random_device{}()};std::ostringstream o;for(int i=0;i<4;i++){if(i)o<<'-';o<<std::hex<<g();}return o.str();}
static std::string json_escape(const std::string&s){std::string r="\"";for(char c:s){if(c=='"'||c=='\\')r+='\\';if(c=='\n')r+="\\n";else r+=c;}return r+'"';}
HttpServer::HttpServer(std::uint16_t p,std::string t):port_(p),token_(std::move(t)){}
int HttpServer::run(){
 const char* dsn=std::getenv("TASKFORGE_PG_DSN"); if(!dsn) dsn="host=postgres port=5432 dbname=taskforge user=taskforge password=change-me-in-dev";
 PgStore db({dsn}); net::io_context io; tcp::acceptor acceptor(io,{tcp::v4(),port_}); Authenticator auth(token_); static std::atomic_uint64_t ids{0};
 for(;;){tcp::socket s(io);acceptor.accept(s);beast::flat_buffer b;http::request<http::string_body> req;beast::error_code ec;http::read(s,b,req,ec);if(ec)continue;
  auto path=std::string(req.target()); http::response<http::string_body> res{http::status::ok,req.version()};res.set(http::field::content_type,"application/json");res.set("X-Request-ID","req-"+std::to_string(++ids));res.set(http::field::access_control_allow_origin,"*");
  try{
   if(req.method()==http::verb::options){res.set(http::field::access_control_allow_headers,"Authorization,Content-Type");res.set(http::field::access_control_allow_methods,"GET,POST,OPTIONS");res.body()="{}";}
   else if(path=="/health")res.body()="{\"status\":\"ok\"}";
   else if(path=="/metrics"){res.set(http::field::content_type,"text/plain; version=0.0.4");res.body()=metrics_.prometheus();}
   else if(path=="/ready"){res.body()="{\"status\":\"ready\"}";}
   else if(!auth.authorize(req[http::field::authorization].to_string())){res.result(http::status::unauthorized);res.body()="{\"error\":\"unauthorized\"}";}
   else if(req.method()==http::verb::post&&path=="/api/v1/jobs"){
     Job j;j.id=uuid();j.execution_id=uuid();j.name="taskforge-job";j.payload=req.body();j.priority=Priority::Normal;j.timeout=std::chrono::milliseconds(30000);auto id=db.create_job(j);metrics_.submitted();res.result(http::status::accepted);res.body()="{\"job_id\":"+json_escape(id)+",\"execution_id\":"+json_escape(j.execution_id)+",\"status\":\"QUEUED\"}";
   } else if(req.method()==http::verb::get&&path=="/api/v1/jobs"){
     auto jobs=db.list_jobs();std::ostringstream o;o<<"{\"jobs\":[";for(size_t i=0;i<jobs.size();++i){if(i)o<<',';o<<"{\"id\":"<<json_escape(jobs[i].id)<<",\"status\":"<<json_escape(to_string(jobs[i].status))<<",\"priority\":"<<json_escape(to_string(jobs[i].priority))<<",\"attempt\":"<<jobs[i].attempt<<"}";}o<<"]}";res.body()=o.str();
   } else if(req.method()==http::verb::get&&path.rfind("/api/v1/jobs/",0)==0){auto id=path.substr(std::string("/api/v1/jobs/").size());auto j=db.get_job(id);if(!j){res.result(http::status::not_found);res.body()="{\"error\":\"job_not_found\"}";}else res.body()="{\"id\":"+json_escape(j->id)+",\"execution_id\":"+json_escape(j->execution_id)+",\"name\":"+json_escape(j->name)+",\"status\":"+json_escape(to_string(j->status))+",\"attempt\":"+std::to_string(j->attempt)+",\"max_attempts\":"+std::to_string(j->max_attempts)+"}";}
   else {res.result(http::status::not_found);res.body()="{\"error\":\"not_found\"}";}
  }catch(const std::exception&e){res.result(http::status::internal_server_error);res.body()="{\"error\":\"internal_error\",\"message\":"+json_escape(e.what())+"}";}
  res.prepare_payload();http::write(s,res,ec);s.shutdown(tcp::socket::shutdown_both,ec);
 }
}
}
