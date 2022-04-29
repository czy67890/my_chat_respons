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
    //timefd_create()���ڴ���һ����ʱ���ź�
    //����create���͸���Ӧ��timerfd�ź�
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
void readTimerFd(int timefd,TimeStamp now){
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

void TimerQueue::cancel(TimerId timerid){
    loop_->runInLoop(std::bind(&TimerQueue::cancelInLoop,this,timerid));
}

//cancel���յ��õĺ���
//��timer��activeTimerList��timer_��ɾ��
void TimerQueue::cancelInLoop(TimerId timerid){
    loop_->assertInLoopThread();
    assert(timer_.size() == activeTimer_.size());
    ActiveTimer timer(timerid.timer_,timerid.sequence_);
    auto iter = activeTimer_.find(timer);
    if(iter != activeTimer_.end()){
        size_t n = timer_.erase(Entry(iter->first->expiration(),iter->first));
        assert(n == 1);
        (void) n;
        delete iter->first;
        activeTimer_.erase(iter);
    } 
    else if(callingExpiredTimers_){
        cancelingTimer_.insert(timer);
    }
    assert(timer_.size() == activeTimer_.size());
}


//�Ƚ����г�ʱ���ȹ���
//ͨ��getExpired��ʵ��
//getExpired������lower_boundʵ�ֵ�
void TimerQueue::handleRead(){
    loop_->assertInLoopThread();
    TimeStamp now(TimeStamp::now());
    readTimerFd(timerfd_,now);

    std::vector<Entry> expired = getExpired(now);

    callingExpiredTimers_ = true;
    cancelingTimer_.clear();
    for(const Entry & it : expired){
        it.second->run();
    }
    callingExpiredTimers_ = false;
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(TimeStamp now){
    assert(timer_.size() == activeTimer_.size());
    std::vector<Entry> expired;
    //UINTPTR_MAX :uintptr�����ֵ
    //uintptr:��ָ���С��ȫһ�������ڽ��κ�ָ��ת�����ٻع�
    Entry sentry(now,reinterpret_cast<Timer *> (UINTPTR_MAX));
    TimerList::iterator end = timer_.lower_bound(sentry);
    assert(end == timer_.end() || now <= end->first);
    //std::cpoy()ʹ�õĵײ���memmove
    //memmove������memcpy
    //memcpy��һ��ȱ�㣬�����ڵ�ַ�����ص���ʱ��ᷢ������
    //��memmove��û���ⷽ�������
    std::copy(timer_.begin(),end,back_inserter(expired));
    timer_.erase(timer_.begin(),end);

    for(const Entry & it:expired){
        ActiveTimer timer(it.second,it.second->sequence());
        size_t n = activeTimer_.erase(timer);
        assert(n == 1);
        (void) n;
    }
    assert(timer_.size() == activeTimer_.size());
    return expired;
}

void TimerQueue::reset(const std::vector<Entry> & expired,TimeStamp now){
    TimeStamp nextExpired;
    for(const Entry & it : expired){
        ActiveTimer timer(it.second,it.second->sequence());
        //����һ�������ʱ���񣬶��Ҳ�����ȡ���Ķ�ʱ��ʱ������
        //����ɾ����timer
        if(it.second->repeat() && cancelingTimer_.find(timer) == cancelingTimer_.end()){
            it.second->restart(now);
            insert(it.second);
        }
        else{
            delete it.second;
        }
    }
    if(!timer_.empty()){
        nextExpired = timer_.begin()->second->expiration();
    }
    if(nextExpired.valid()){
        resetTimerfd(timerfd_,nextExpired);
    }
}

//���������ɵ�TimerQueue
//ֻ��Ҫ�Աȿ�ͷ����
bool TimerQueue::insert(Timer * timer){
    loop_->assertInLoopThread();
    assert(timer_.size() == activeTimer_.size());
    bool earliestChanged = false;
    TimeStamp when = timer->expiration();
    TimerList::iterator it = timer_.begin();
    if(it == timer_.end() ||when<it->first){
        earliestChanged = true;
    }
    //ͨ�����������ֶ�������������
    {
        std::pair<TimerList::iterator,bool> result = 
        timer_.insert(std::move(Entry(when,timer)));
        assert(result.second);
        (void) result;
    }
    {   
        //insert����һ��pair
        //firstΪһ��ietratorָ���½�ֵ
        //secondΪһ��ָʾ�Ƿ�ɹ��ı���
        std::pair<ActiveTimerSet::iterator,bool> result = 
        activeTimer_.insert(std::move(ActiveTimer(timer,timer->sequence())));
        assert(result.second);
        //������д����Ϊ�˷�ֹ����������
        //ʵ���ϲ�û������
        (void) result;
    }
}


