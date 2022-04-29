#pragma once
#include"../base/nocopyable.h"
struct tcp_info;
namespace czy{
namespace net{
class InetAddress;
class Socket :nocopyable{
public:
//使用explict 关键字防止其构造函数隐式转换
    explicit Socket(int sockfd): m_sockfd(sockfd){ }
    ~Socket();
    int fd() const{ return m_sockfd;}
    bool get_tcp_info(struct tcp_info*) const;
    bool get_tcp_info_string(char *buf,int len) const;
    void bind_addr(const InetAddress &local_addr);
    void listen();
    int accept(InetAddress *peeraddr);
    void shotdown_write();
    void set_tcp_nodelay(bool on);
    void set_reuseable_addr(bool on);
    void set_reuseadble_port(bool on);
    void set_keep_alive(bool on);
private:
    int m_sockfd;
};
}
}