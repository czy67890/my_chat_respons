#include"tcp_server.h"

#include"../base/logging.h"
#include"acceptor.h"
#include"event_loop.h"
#include"eventloop_thread.h"
#include"socketops.h"
#include"eventloop_threadpool.h"
#include<stdio.h>
#include<memory>

using namespace czy;
using namespace czy::net;

TcpServer::TcpServer(EventLoop* loop,
                     const InetAddress& listenAddr,
                     const string& nameArg,
                     Option option)
  : loop_(CHECK_NOTNULL(loop)),
    ipPort_(listenAddr.toIpPort()),
    name_(nameArg),
    acceptor_(new Acceptor(loop, listenAddr, option == kReusePort)),
    threadPool_(new EventLoopThreadPool(loop, name_)),
    connectionCallback_(defaultConnectionCallback),
    messageCallback_(defaultMessageCallback),
    nextConnId_(1)
{
    acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection,this,_1,_2));
}

TcpServer::~TcpServer(){
    loop_->assertInLoopThread();
    LOG_TRACE<<"TcpServer::~TcpServer ["<<name_<<" ] desstructing";
    for(auto & item:connections_){
        TcpConnectionPtr conn(item.second);
        item.second.reset();
        conn->getLoop()->runInLoop(std::bind(&TcpConnection::connectDestory,conn));
    }
}

void TcpServer::setThreadNum(int numThread){
    assert(numThread >= 0);
    threadPool_->setThreadNum(numThread);
}

void TcpServer::start(){
    if(started_.get_and_set(1) == 0){
        //threadpool创建线程之后
        //会因为自己的loop而阻塞
        
        threadPool_->start(threadInitCallback_);
        assert(!acceptor_->listening());
        loop_->runInLoop(std::bind(&Acceptor::listen,get_pointer(acceptor_)));
    }
}

void TcpServer::newConnection(int sockfd,const InetAddress & addr){
    loop_->assertInLoopThread();
    EventLoop *ioloop = threadPool_->getNextLoop();
    char buf[64];
    snprintf(buf,sizeof(buf),"-%s#%d",ipPort_.c_str(),nextConnId_);
    ++nextConnId_;
    string conname = name_ + buf;
    LOG_INFO<<"TcpSerever::new Connection["<<name_<<
    "] - new connection["<<conname<<"]from "<<addr.toIpPort();
    InetAddress localAddr(sockets::getLocalAddr(sockfd));
    //efectivec++中的推荐用法
    //而不是让conn指向new
    //这样是异常安全的
    TcpConnectionPtr conn(std::make_shared<TcpConnection>(ioloop,conname,sockfd,localAddr,addr));
    connections_[conname] = conn;
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    //注册到TcpConnection中
    //在TcpConnection关闭时
    //从自己的map中删除相关信息
    conn->setCloseCallback(std::bind(&TcpServer::removeConnection,this,_1));
    ioloop->runInLoop(std::bind(&TcpConnection::connectEstablished,conn));
}

void TcpServer::removeConnection(const TcpConnectionPtr & conn){
    loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop,this,conn));
}

//删除自己map中的相关connection
//并且调用Tcpconnection中的destory销毁链接
void TcpServer::removeConnectionInLoop(const TcpConnectionPtr & conn){
    loop_->assertInLoopThread();
    LOG_INFO<<"TcpServer::removeConnectionInLoop ["<<name_<<
    "] - connection "<<conn->name();
    size_t n = connections_.erase(conn->name());
    (void) n;
    assert(n == 1);
    EventLoop * ioLoop = conn->getLoop();
    ioLoop->queueInLoop(std::bind(&TcpConnection::connectDestory,conn));
}