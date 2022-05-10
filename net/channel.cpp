#include"channel.h"
#include"../base/logging.h"
#include"../net/event_loop.h"

#include<sstream>
#include<poll.h>

using namespace czy;
using namespace czy::net;

const int Channel::KNoneEvent = 0;
const int Channel::KReadEvent = POLLIN|POLLPRI;
const int Channel::KWriteEvent = POLLOUT;
Channel::Channel(EventLoop *loop,int fd)
 :fd_(fd),loop_(loop),events_(0),revents_(0),index_(0),logHup_(true),
 tied_(false),eventHandling_(false),addToLoop_(false)
{ }

Channel::~Channel(){
    assert(!eventHandling_);
    assert(!addToLoop_);
    if(loop_->isInLoopThread()){
        assert(!loop_->hasChannel(this));
    }
}

void Channel::tie(const std::shared_ptr<void> &obj){
    //典型的使用shared_ptr来管理的例子
    //tie_是一个weak_ptr
    //只能和shared_ptr连用
    tie_ = obj;
    tied_ = true;
}

void Channel::update(){
    addToLoop_ = true;
    loop_->updateChannel(this);
}

void Channel::remove(){
    assert(isNoneEvent());
    addToLoop_ = false;
    loop_->removeChannel(this);
}

void Channel::handleEvent(TimeStamp recvtime){
    std::shared_ptr<void> guard;
    if(tied_){
        //handleevent时提升自己的tie weak_ptr
        //延长对象的生命周期，使得不被析构而造成coredump
        guard = tie_.lock();    
        if(guard){
            handleEventWithGuard(recvtime);
        }
    }
    else{
        handleEventWithGuard(recvtime);
    }
}

void Channel::handleEventWithGuard(TimeStamp recvtime){
    eventHandling_ = true;
    LOG_TRACE<<reventsToString();
    if((revents_ & POLLHUP) && !(revents_& POLLIN)){
        if(logHup_){
            LOG_WARN<<"fd = "<<fd_<<" Channel handling events POLLHUP";
        }
        if(closeCallback_)closeCallback_();
    }

    //POLLNVAL在文件描述符未打开或者打开的是一个空值的时候发生
    if(revents_ & POLLNVAL){
        LOG_WARN<<"fd = "<<fd_<<" Channel handling events POLLNVAL";
    }
    if(revents_ &( POLLERR||POLLNVAL)){
        if(errorCallback_){
            errorCallback_();
        }
    }
    //POLLPRI提醒有紧急事件读
    //POLLRDHUP提醒对端已经关闭
    if(revents_ & (POLLIN|POLLPRI|POLLRDHUP)){
        if(readCallback_){
            readCallback_(recvtime);
        }
    }
    if(revents_& POLLOUT){
        if(writeCallback_)writeCallback_();
    }
    eventHandling_ = false;
}
std::string Channel::reventsToString() const{
    return eventsToString(fd_,revents_);
}
std::string Channel::eventsToString() const{
    return eventsToString(fd_,events_);
}
std::string Channel::eventsToString(int fd,int ev){
    std::ostringstream oss;
    oss<<fd<<":";
    if(ev&POLLIN){
        oss<<"IN";
    }
    if(ev&POLLPRI){
        oss<<"PRI";
    }
    if(ev&&POLLOUT){
        oss<<"OUT";
    }
    if(ev&&POLLHUP){
        oss<<"HUP";
    }
    if(ev&POLLRDHUP){
        oss<<"RDHUP";
    }
    if(ev&POLLERR){
        oss<<"ERR";
    }
    if(ev&POLLNVAL){
        oss<<"NVAL";
    }
    return oss.str();
}



