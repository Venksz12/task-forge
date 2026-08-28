#include "taskforge/auth.hpp"
namespace taskforge {
bool constant_time_equal(const std::string&a,const std::string&b){unsigned char d=static_cast<unsigned char>(a.size()^b.size());auto n=a.size()>b.size()?a.size():b.size();for(std::size_t i=0;i<n;++i){auto x=i<a.size()?static_cast<unsigned char>(a[i]):0;auto y=i<b.size()?static_cast<unsigned char>(b[i]):0;d|=x^y;}return d==0;}
bool Authenticator::authorize(const std::string& h)const{return h.rfind("Bearer ",0)==0&&constant_time_equal(h.substr(7),token_);}
}
