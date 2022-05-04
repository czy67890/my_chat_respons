#include"tcp_client.h"
#include"../base/logging.h"
#include"connector.h"
#include"event_loop.h"
#include"socketops.h"

#include<stdio.h>

using namespace czy;
using namespace czy::net;

namespace czy{
namespace net{
//Impl的手法
//这样能够提升编译的效率

namespace detail{

void removeConnection(EventLoop *loop,const TcpConnectionPtr & conn){
    loop->queueInLoop(std::bind(&TcpConnection::connectDestory,conn));
}

void removeConnector(const  ConnectorPtr & connector){

}
}
}
}

TcpClient::TcpClient(EventLoop *loop,const InetAddress & serverAddr,const string &nameArg)
  :loop_(CHECK_NOTNULL(loop)),
    connector_(new Connector(loop, serverAddr)),
    name_(nameArg),
    connectionCallback_(defaultConnectionCallback),
    messageCallback_(defaultMessageCallback),
    retry_(false),
    connect_(true),
    nextConnId_(1)
{   
    //第二个this是因为需要给类的非静态成员函数指定调用者
    //非常量的引用初始化必须为左值
    connector_->setNewConnectionCallback(std::bind(&TcpClient::newConnection,this,_1));
    LOG_INFO<<"TcpClient:TcpClient["<<name_<<"] - connector"<<get_pointer(connector_);
}

TcpClient::~TcpClient(){
    LOG_INFO<<"TcpClinet::~TcpCLient["<<name_<<"] - connector "<<get_pointer(connector_);
    TcpConnectionPtr conn;
    bool unique = false;
    {
        MutexGroud lock(mutex_);
        unique = connection_.unique();
        conn = connection_;
    }
    if(conn){
        assert(loop_ == conn->getLoop());
        //构建cbfunction
        CloseCallback cb = std::bind(&detail::removeConnection,loop_,_1);
        //将TcpConnectoion的closecallback改成TcpconnectionDestory
        loop_->runInLoop(std::bind(&TcpConnection::setCloseCallback,conn,cb));
        if(unique){
            conn->forceClose();
        }
    }
    else {
        connector_->stop();
        loop_->runAfter(1,std::bind(&detail::removeConnector,connector_));
    }
}

void TcpClient::connect(){
    LOG_INFO<<"TcpClient::connect["<<name_<<"] - connecting to"<<connector_->serverAddress().toIpPort();
    connect_ = true;
    connector_->start();
}

void TcpClient::disconnect(){
    connect_ = false;
   {
       MutexGroud lock(mutex_);
       if(connection_){
           connection_->shutdown();
       }
   }
}

void TcpClient::stop(){
    connect_ = false;
    connector_->stop();
}

void TcpClient::newConnection(int sockfd){
    loop_->assertInLoopThread();
    InetAddress peerAddr(sockets::getPeerAddr(sockfd));
    char buf[32];
    snprintf(buf,sizeof(buf),":%s#%d",peerAddr.toIpPort().c_str(),nextConnId_);
    ++nextConnId_;
    string conname = name_ + buf;
    InetAddress localAddr(sockets::getLocalAddr(sockfd));
    TcpConnectionPtr conn (new TcpConnection(loop_,conname,sockfd,localAddr,peerAddr));
    conn->setCloseCallback(std::bind(&TcpClient::removeConnection,this,_1));
    conn->setConnectionCallback(connectionCallback_);
  conn->setMessageCallback(messageCallback_);
  conn->setWriteCompleteCallback(writeCompleteCallback_);
    {
        MutexGroud lock(mutex_);
        connection_ = conn;
    }
    conn->connectEstablished();
}

void TcpClient::removeConnection(const TcpConnectionPtr& conn){
    loop_->assertInLoopThread();
    assert(loop_ == conn->getLoop());
    {
        MutexGroud lock(mutex_);
        assert(connection_ == conn);
        //shared_ptr的reset方法将管理的资源减一
        connection_.reset();
    }
    loop_->queueInLoop(std::bind(&TcpConnection::connectDestory,conn));
    if(retry_ && connect_){
        LOG_INFO<<"TcpClient::connect["<<name_<<"] - Reconnecting to"<<connector_->serverAddress().toIpPort();
        connector_->restart();
    }
}


