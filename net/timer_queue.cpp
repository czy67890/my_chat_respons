#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include"timer_queue.h"
#include"../base/logging.h"
#include"event_loop.h"
#include"timer.h"
#include"timer_id.h"
#include<sys/timerfd.h>
#include<unistd.h>
namespace czy{
namespace net{
namespace detail{

int createTimerfd(){
    //timefd_create()用于创建一个计时器信号
    //会在create后发送给相应的timerfd信号
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC,TFD_NONBLOCK|TFD_CLOEXEC);
    if(timerfd < 0){
        LOG_SYSFATAL<<"failed to time_create";
    }
    return timerfd;
}

struct timespec howMuchTimeFromNow(TimeStamp when){
    int64_t micro_second = when.micro_since_epoch() - TimeStamp::now().micro_since_epoch();
    if(micro_second < 100){
        micro_second = 100;
    }
    struct timespec spec;
    spec.tv_sec = static_cast<time_t>(micro_second / TimeStamp::k_micro_seconds_persecond);
    spec.tv_nsec = static_cast<long>
    ((micro_second % TimeStamp::k_micro_seconds_persecond) *1000);
    return spec;
}
void readTimeFd(int timefd,TimeStamp now){
    uint64_t howmany;
    ssize_t n = ::read(timefd,&howmany,sizeof(howmany));
    LOG_TRACE<<"TimerQueue::handleRead() "<<howmany<<"at"<<now.to_string();
    if(n != howmany){
        LOG_ERROR<<"TimerQueue:handleRead() reads"<<n<<" bytes instead of 8";
    }
}
void resetTimerfd(int timefd,TimeStamp expiration){
    struct itimerspec newVal;
    struct itimerspec oldVal;
    mem_zero(&newVal,sizeof(newVal));    
    mem_zero(&oldVal,sizeof(oldVal));
    newVal.it_value = howMuchTimeFromNow(expiration);
    int ret = ::timerfd_settime(timefd,0,&newVal,&oldVal);
    if(ret){
        LOG_SYSERR<<"timerfd_settime";
    }
}
}
}
}
using namespace czy;
using namespace czy::net;
using namespace czy::net::detail;

czy::net::TimerQueue::TimerQueue(EventLoop* loop)
 :loop_(loop),timerfd_(createTimerfd()),
 timerfdChannel_(loop,timerfd_),timer_(),
 callingExpiredTimers_(false)
{
    timerfdChannel_.setReadCallback(std::bind(&TimerQueue::handleRead,this));
    timerfdChannel_.enableReading();
}

TimerQueue::~TimerQueue(){
    timerfdChannel_.disAbleAll();
    timerfdChannel_.remove();
    ::close(timerfd_);
    for(const Entry& timer:timer_){
        delete timer.second;
    }
}

TimerId TimerQueue::addTimer(TimerCallback cb,TimeStamp when,double interval){
    Timer *timer = new Timer(std::move(cb),when,interval);
    loop_->runInLoop(std::bind(&TimerQueue::addTimerInLoop,this,timer));
    return TimerId(timer,timer->sequence());
}
void TimerQueue::cancel(TimerId timerid){
    loop_->runInLoop(std::bind(&TimerQueue::cancelInLoop,this,timerid));
}

void TimerQueue::addTimerInLoop(Timer * timer){
    loop_->assertInLoopThread();
    bool eariliestChanged = insert(timer);
    if(eariliestChanged){
        resetTimerfd(timerfd_,timer->expiration());
    }
}



