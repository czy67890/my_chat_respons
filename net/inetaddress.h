#pragma once
#include"../base/string_piece.h"
#include<netinet/in.h>
#include"../base/nocopyable.h"
namespace czy{
namespace net{
namespace sockets{
const struct sockaddr * sockaddr_cast(const struct sockaddr_in6 *addr);
}//namespace sockets

class InetAddress :  public nocopyable{
public:
explicit InetAddress(uint16_t port = 0,bool loop_back_only = false,bool ipv6 = false);
InetAddress(StringArg ip,uint16_t port,bool ipv6 = false);
explicit InetAddress(const struct sockaddr_in& addr) : addr_(addr){
}
explicit InetAddress(const struct sockaddr_in6& addr) : addr6_(addr){
}
sa_family_t family() const{return addr_.sin_family;}
std::string tpIp () const;
std::string toIpPort() const;
uint16_t port() const;
const struct sockaddr* getSockAddr() const{return czy::net::sockets::sockaddr_cast(&addr6_);}
void setSockAddrInet6(const struct sockaddr_in6 &addr6){
    addr6_ = addr6;
}
uint32_t ipv4NetEndian()const;
uint16_t portNetEndian() const;
static bool resolve(StringArg hostname,InetAddress* reslut);
void setScopeId(uint32_t scope_id);
private:
    //union用于节省空间
    //并且使得意义明确
    union 
    {
        struct sockaddr_in addr_;
        struct sockaddr_in6 addr6_;
    };
};
}

}