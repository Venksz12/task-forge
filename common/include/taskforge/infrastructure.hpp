#pragma once
#include "taskforge/models.hpp"
#include <boost/asio.hpp>
#include <libpq-fe.h>
#include <string>
#include <vector>
#include <mutex>
#include <optional>
#include <chrono>
namespace taskforge {
struct DbConfig { std::string conninfo; };
class PgStore {
  PGconn* conn_{}; std::mutex mutex_;
public:
  explicit PgStore(const DbConfig& cfg); ~PgStore();
  PgStore(const PgStore&)=delete; PgStore& operator=(const PgStore&)=delete;
  std::string create_job(const Job& job);
  std::optional<Job> get_job(const std::string& id);
  std::vector<Job> list_jobs(int limit=100);
  bool update_status(const std::string& id, JobStatus status, const std::string& worker="");
  bool record_attempt(const Job& job, const std::string& worker, const std::string& status, const std::string& error="");
};
class RedisClient {
  boost::asio::io_context io_; boost::asio::ip::tcp::socket socket_; std::mutex mutex_;
  std::string host_; uint16_t port_;
  std::string command(const std::vector<std::string>& args);
public:
  RedisClient(std::string host="redis", uint16_t port=6379); void setex(const std::string&, int, const std::string&); bool exists(const std::string&);
};
class NatsClient {
  boost::asio::io_context io_; boost::asio::ip::tcp::socket socket_; std::mutex mutex_; std::string host_; uint16_t port_; bool subscribed_{false};
  void connect(); std::string read_line();
public:
  NatsClient(std::string host="nats", uint16_t port=4222); ~NatsClient();
  void publish(const std::string& subject,const std::string& payload);
  std::optional<std::pair<std::string,std::string>> next(const std::string& subject);
};
}
