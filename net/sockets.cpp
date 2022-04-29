#include<netinet/in.h>
#include<netinet/tcp.h>
#include<stdio.h>

#include"sockets.h"
#include"../base/logging.h"
#include"inetaddress.h"
#include "socketops.h"

using namespace czy;
using namespace czy::net;
Socket::~Socket(){
    sockets::close( m_sockfd );
}

bool Socket::get_tcp_info(struct tcp_info *tcpi) const{
    socklen_t len = sizeof(*tcpi);
    mem_zero(tcpi,len);
    return ::getsockopt(m_sockfd,SOL_TCP,TCP_INFO,tcpi,&len) == 0;
} 

bool Socket::get_tcp_info_string(char *buf,int len ) const{
    struct tcp_info tcpi;
    bool ok = get_tcp_info(&tcpi);
    if(ok){
    snprintf(buf, len, "unrecovered=%u "
             "rto=%u ato=%u snd_mss=%u rcv_mss=%u "
             "lost=%u retrans=%u rtt=%u rttvar=%u "
             "sshthresh=%u cwnd=%u total_retrans=%u",
             tcpi.tcpi_retransmits,  // Number of unrecovered [RTO] timeouts
             tcpi.tcpi_rto,          // Retransmit timeout in usec
             tcpi.tcpi_ato,          // Predicted tick of soft clock in usec
             tcpi.tcpi_snd_mss,
             tcpi.tcpi_rcv_mss,
             tcpi.tcpi_lost,         // Lost packets
             tcpi.tcpi_retrans,      // Retransmitted packets out
             tcpi.tcpi_rtt,          // Smoothed round trip time in usec
             tcpi.tcpi_rttvar,       // Medium deviation
             tcpi.tcpi_snd_ssthresh,
             tcpi.tcpi_snd_cwnd,
             tcpi.tcpi_total_retrans);  // To
    }
    return ok;
}

void Socket::bind_addr(const InetAddress &addr){
    sockets::bindOrDie(m_sockfd,addr.getSockAddr());
}

void Socket::listen(){
    sockets::listenOrDie(m_sockfd);
}

int Socket::accept(InetAddress *peeraddr){
    struct sockaddr_in6 addr;
    mem_zero(&addr,sizeof(addr));
    int connfd = sockets::accept(m_sockfd,&addr);
    if(connfd >= 0){
        peeraddr->setSockAddrInet6(addr);
    }
    return connfd;
}

void Socket::shotdown_write(){
    sockets::shutdownWrite(m_sockfd);
}
//设置TCP的无延迟选项
//防止小数据包对网络的影响
void Socket::set_tcp_nodelay(bool on){
    int optval = on?1:0;
    ::setsockopt(m_sockfd,IPPROTO_TCP,TCP_NODELAY,
    &optval,static_cast<socklen_t> (sizeof(optval))
    );
}

void Socket::set_reuseable_addr(bool on){
    int optval = on ? 1 : 0;
  ::setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEADDR,
               &optval, static_cast<socklen_t>(sizeof optval));
}

void Socket::set_reuseadble_port()
