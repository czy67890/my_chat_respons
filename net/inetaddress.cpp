#include"inetaddress.h"
#include"../base/logging.h"
#include"endian.h"
#include"socketops.h"
#include<netdb.h>
#include<netinet/in.h>
#pragma GCC diagnostic ignored "-Wold-style-cast"
static const in_addr_t kInaddrAny = INADDR_ANY;
static const in_addr_t kInaddrLoopback = INADDR_LOOPBACK;
#pragma GCC diagnostic error "-Wold-style-cast"
namespace czy{
namespace net{
InetAddress::InetAddress(uint16_t portArg, bool loopbackOnly, bool ipv6){
    if(ipv6){
        mem_zero(&addr6_,sizeof(addr6_));
        addr6_.sin6_family = AF_INET6;
        //loopbackonly设置是否只监听局域网的内容
        in6_addr ip = loopbackOnly?in6addr_loopback:in6addr_any;
        addr6_.sin6_addr = ip;
        //一定要注意网络字节转换的问题
        addr6_.sin6_port = sockets::hostToNetwork16(portArg);
    }
    else{
        mem_zero(&addr_,sizeof(addr_));
        addr_.sin_family = AF_INET;
        addr_.sin_port = sockets::hostToNetwork16(portArg);
        addr_.sin_addr.s_addr = sockets::hostToNetwork32(static_cast<in_addr_t>(loopbackOnly?kInaddrLoopback:kInaddrAny));
    }
}
InetAddress::InetAddress(StringArg ip,uint16_t portArg,bool ipv6){
    if(ipv6 || strchr(ip.c_str(),':')){
        mem_zero(&addr6_,sizeof(addr6_));
        sockets::fromIpPort(ip.c_str(),portArg,&addr6_);
    }
    else{
        mem_zero(&addr_,sizeof(addr_));
        sockets::fromIpPort(ip.c_str(),portArg,&addr_);
    }
}
std::string InetAddress::toIpPort() const{
    char buf[64] = "";
    sockets::toIpPort(buf,sizeof(buf),getSockAddr());
    return buf;
}

uint32_t InetAddress::ipv4NetEndian() const{
    assert(family() == AF_INET);
    return addr_.sin_addr.s_addr;
}

uint16_t InetAddress::port() const{
    return sockets::networkToHost16(portNetEndian());
}
static __thread char  t_resloveBuffer[64*1024];

//发送DNS请求
//并且解析到out
bool InetAddress::resolve(StringArg hostname,InetAddress* out){
    assert(out != NULL);
    struct hostent hent;
    struct hostent* he = NULL;
    int herrno = 0;
    mem_zero(&hent,sizeof(hent));
    //发送DNS请求，获得IP地址
    int ret = gethostbyname_r(hostname.c_str(),&hent,t_resloveBuffer,sizeof(t_resloveBuffer),&he,&herrno);
    if(ret == 0 & he != NULL){
        assert(he->h_addrtype == AF_INET &&he->h_length == sizeof(uint32_t));
        out->addr_.sin_addr = *reinterpret_cast<struct in_addr*>(he->h_addr);
        return true;
    }
    else{
        if(ret){
            LOG_SYSERR<<"InetAddress::reslove";
        }
        return false;
    }
}
void InetAddress::setScopeId(uint32_t scope_id){
    if(family() == AF_INET6){
        addr6_.sin6_scope_id = scope_id;
    }
}
}
}