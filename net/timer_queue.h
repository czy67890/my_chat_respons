#pragma once
#include<set>
#include<vector>
#include"../base/locker.h"
#include"../base/time_stamp.h"
#include"callbacks.h"
#include"channel.h"
#include"../base/nocopyable.h"
namespace czy{
namespace net{

class EventLoop;
class Timer;
class TimerId;
//TimerQueueֻ��һ��besteffort delivery
//����֤���еĻص�����׼ʱ����
class TimerQueue: nocopyable{
public:
    explicit TimerQueue(EventLoop * loop);
    ~TimerQueue();
    TimerId addTimer(TimerCallback cb,TimeStamp when,double interval);
    void cancel(TimerId timerId);
private:
    typedef std::pair<TimeStamp,Timer*> Entry;
    typedef std::set<Entry> TimerList;
    typedef std::pair<Timer *,int64_t> ActiveTimer;
    typedef std::set<ActiveTimer> ActiveTimerSet;
    void addTimerInLoop(Timer* timer);
    void cancelInLoop(TimerId timerId);
    void handleRead();
    std::vector<Entry> getExpired(TimeStamp now);
    void reset(const std::vector<Entry>& expired,TimeStamp now);
    bool insert(Timer* timer);
    EventLoop *loop_;
    const int timerfd_;
    Channel timerfdChannel_;
    TimerList timer_;
    ActiveTimerSet activeTimer_;
    bool callingExpiredTimers_;
    ActiveTimerSet cancelingTimer_;
};
}
}