#include"connector.h"
#include"../base/logging.h"
#include"channel.h"
#include"event_loop.h"
#include"socketops.h"

#include<errno.h>

//用来代理所有的connect请求
using namespace czy;
using namespace czy::net;

const int Connector::KMaxRetryDelayMs;
Connector::Connector(EventLoop* loop, const InetAddress& serverAddr)
  : loop_(loop),
    serverAddr_(serverAddr),
    connect_(false),
    state_(KDisconnected),
    retryDelayMs_(KInitRetryDelayMs)
{
  LOG_DEBUG << "ctor[" << this << "]";
}

Connector::~Connector(){
    LOG_DEBUG<<"dtor["<<this<<"]";
    assert(!channel_);
}

void Connector::start(){
    connect_ = true;

    //safe version
    loop_->runInLoop(std::bind(&Connector::startInLoop,shared_from_this()));
}
void Connector::startInLoop(){
    loop_->assertInLoopThread();
    assert(state_ == KDisconnected);
    if(connect_){
        connect();
    }
    else{
        LOG_DEBUG<<"do not connect";
    }
}

void Connector::stop(){
    connect_ = false;

    //safe version
    loop_->queueInLoop(std::bind(&Connector::stopInLoop,shared_from_this()));
}


void Connector::connect(){
  int sockfd = sockets::createNonblockingOrDie(serverAddr_.family());
  int ret = sockets::connect(sockfd, serverAddr_.getSockAddr());
  int savedErrno = (ret == 0) ? 0 : errno;
  switch (savedErrno)
  {
    case 0:
    case EINPROGRESS:
    case EINTR:
    case EISCONN:
      connecting(sockfd);
      break;

    case EAGAIN:
    case EADDRINUSE:
    case EADDRNOTAVAIL:
    case ECONNREFUSED:
    case ENETUNREACH:
      retry(sockfd);
      break;

    case EACCES:
    case EPERM:
    case EAFNOSUPPORT:
    case EALREADY:
    case EBADF:
    case EFAULT:
    case ENOTSOCK:
      LOG_SYSERR << "connect error in Connector::startInLoop " << savedErrno;
      sockets::close(sockfd);
      break;

    default:
      LOG_SYSERR << "Unexpected error in Connector::startInLoop " << savedErrno;
      sockets::close(sockfd);
      // connectErrorCallback_();
      break;
  } 
}

void Connector::restart(){
    loop_->assertInLoopThread();
    setState(KDisconnected);
    retryDelayMs_ = KInitRetryDelayMs;
    connect_ = true;
    startInLoop();
}

void Connector::connecting(int sockfd){
    setState(KConnecting);
    assert(!channel_);
    //设置channel的回调函数
    //只有在回调时才会get_shared_from_this
    //这样不会造成指针过多
    channel_->setWriteCallback(std::bind(&Connector::handleWrite,shared_from_this()));
    channel_->setErrorCallback(std::bind(&Connector::handleEror,shared_from_this()));
    channel_->enableWriting();
}

int Connector::removeAndReserChannel(){
    channel_->disAbleAll();
    channel_->remove();
    int sockfd = channel_->fd();
    loop_->queueInLoop(std::bind(&Connector::resetChannel,shared_from_this()));
    return sockfd;
}

void Connector::resetChannel(){
    channel_.reset();
}

void Connector::handleWrite(){
    LOG_TRACE<<"Connector::handleWrite "<<state_;
    if(state_ == KConnecting){
        int sockfd = removeAndReserChannel();
        int err = sockets::getSocketError(sockfd);
        if(err){
            LOG_WARN<<"Connection::handleWrite  -Self connect";
            retry(sockfd);
        }
        else{
            setState(KConnected);
            if(connect_){
                newConnectionCallback_(sockfd);
            }
            else{
                sockets::close(sockfd);
            }
        }
    }
    else{
        assert(state_ == KDisconnected);
    }
}

void Connector::handleEror(){
    LOG_ERROR<<"Connector::handleError state = "<<state_;
    if(state_ == KConnecting){
        int sockfd = removeAndReserChannel();
        int err = sockets::getSocketError(sockfd);
        LOG_TRACE<<"SO_ERROR = "<<err<<" "<<strerror_tl(err);
        retry(sockfd);
    }
}

void Connector::retry(int sockfd){
    sockets::close(sockfd);
    setState(KDisconnected);
    if(connect_){
        LOG_INFO << "Connector::retry - Retry connecting to " << serverAddr_.toIpPort()
             << " in " << retryDelayMs_ << " milliseconds. ";
        loop_->runAfter(retryDelayMs_/1000.0,
                    std::bind(&Connector::startInLoop, shared_from_this()));
        retryDelayMs_ = std::min(retryDelayMs_ * 2, KMaxRetryDelayMs);
    }
    else{
        LOG_DEBUG<<"do not connect";
    }
}





