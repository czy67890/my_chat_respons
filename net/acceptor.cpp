#include"acceptor.h"
#include"../base/logging.h"
#include"event_loop.h"
#include"inetaddress.h"
#include"socketops.h"

#include<errno.h>
#include<fcntl.h>
#include<unistd.h>

using namespace czy;
using namespace czy::net;

Acceptor::Acceptor(EventLoop *loop,const InetAddress &listenAddr,bool reuseport)
 :loop_(loop),
    acceptSocket_(sockets::createNonblockingOrDie(listenAddr.family())),
    //在这里给tcpserver的accept做好channel分发
    acceptChannel_(loop, acceptSocket_.fd()),
    listening_(false),
    //O_CLOEXEC:
    //在执行exec系统调用的时候，关闭文件描述符
    //防止父进程的文件描述符传递给子进程，导致子进程访问没有权限的文件
    idleFd_(::open("/dev/null", O_RDONLY | O_CLOEXEC))
{
    assert(idleFd_ >= 0);
    acceptSocket_.set_reuseable_addr(true);
    acceptSocket_.set_reuseadble_port(reuseport);
    acceptSocket_.bind_addr(listenAddr);
    acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead,this));
}

Acceptor::~Acceptor(){
    acceptChannel_.disAbleAll();
    acceptChannel_.remove();
    ::close(idleFd_);
}

void Acceptor::listen(){
    loop_->assertInLoopThread();
    listening_ = true;
    acceptSocket_.listen();
    acceptChannel_.enableReading();
}

//Acceptor的职责，监听新的到来链接
//在调用accept接受这个新连接
//确保中间没有发生问题后
//调用newConnectionCallback_
//这里被TcpServer设置为其中调用其中的方法newConnection
//并加入TcpServer的map中
void Acceptor::handleRead(){
    loop_->assertInLoopThread();
    InetAddress peerAddr;
    int connfd = acceptSocket_.accept(&peerAddr);
    if(connfd >= 0){
        if(newConnectionCallback_){
            newConnectionCallback_(connfd,peerAddr);
        }
        else{
            sockets::close(connfd);
        }
    }
    else{
        LOG_SYSERR<<"in Acceptor:: handleRead";
        //EMFILE表示打开文件描述符达到最大
        if(errno == EMFILE){
            ::close(idleFd_);
            idleFd_ = ::accept(acceptSocket_.fd(),NULL,NULL);
            ::close(idleFd_);
            idleFd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
        }
    }
}