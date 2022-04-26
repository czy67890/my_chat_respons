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
    //���͵�ʹ��shared_ptr�����������
    //tie_��һ��weak_ptr
    //ֻ�ܺ�shared_ptr����
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
    if(revents_ & POLLNVAL){
        LOG_WARN<<"fd = "<<fd_<<" Channel handling events POLLNVAL";
    }
}

