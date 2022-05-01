#include"tcp_connection.h"
#include"../base/weak_callback.h"
#include"../base/logging.h"
#include"channel.h"
#include"event_loop.h"
#include"sockets.h"
#include"socketops.h"
#include<errno.h>

using namespace czy;
using namespace czy::net;

void czy::net::defaultConnectionCallback(const TcpConnectionPtr& conn ){
     LOG_TRACE << conn->localAddress().toIpPort() << " -> "
            << conn->peerAddress().toIpPort() << " is "
            << (conn->connected() ? "UP" : "DOWN");
  // do not call conn->forceClose(), because some users want to register message callback only.
}


void czy::net::defaultMessageCallback(const TcpConnectionPtr &conn,Buffer * buf,TimeStamp){
    buf->retriveAll();
}

TcpConnection::TcpConnection(EventLoop *loop
                            ,const string &nameArg,
                            int sockfd,
                            const InetAddress & locadAddress,
                            const InetAddress & peerAddress)
:loop_(loop),name_(nameArg),state_(KConnecting),
reading_(true),socket_(new Socket(sockfd)),channel_(new Channel(loop,sockfd)),
localAddress_(locadAddress),peerAddress_(peerAddress),highWaterMark_(64*1024*1024)                            
{   //使用std::bind()确保一定是由本对象来调用该回调函数
    channel_->setCloseCallback(std::bind(&TcpConnection::handleClose,this));
    channel_->setErrorCallback(std::bind(&TcpConnection::handleError,this));
    channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite,this));
    channel_->setReadCallback(std::bind(&TcpConnection::handleRead,this,_1));
    LOG_DEBUG<<"TcpConnection::ctor["<<name_<<"] at"<<this<<" fd = "<<sockfd;
    socket_->set_keep_alive(true);    
}

TcpConnection::~TcpConnection()
{
  LOG_DEBUG << "TcpConnection::dtor[" <<  name_ << "] at " << this
            << " fd=" << channel_->fd()
            << " state=" << stateTostring();
  assert(state_ == KDisconnected);
}

bool TcpConnection::getTcpInfo(struct tcp_info* info) const{
    
}