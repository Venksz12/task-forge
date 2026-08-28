#pragma once
#include <string>
namespace taskforge { bool constant_time_equal(const std::string&,const std::string&); class Authenticator{std::string token_;public:explicit Authenticator(std::string token):token_(std::move(token)){}bool authorize(const std::string&)const;}; }
