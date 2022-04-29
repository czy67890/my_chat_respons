#pragma once
#include"../base/atomic.h"
#include"../base/Timestamp.h"
#include"callbacks.h"
//定时器类

//拥有回调函数

//超时时间
namespace czy{

namespace net{
class Timer:nocopyable{

public:
    Timer(TimerCallback cb,TimeStamp when,double interval)
     :callback_(cb),expiration_(when),interval_(interval),
     repeat_(interval>0.0),sequence_(s_numCreated_.increment_and_get()) 
    { }
    void run() const{
        callback_();
    }
    TimeStamp expiration() const{return expiration_;} 
    bool repeat() const { return repeat_;}
    int64_t sequence() const{ return sequence_;}
    void restart(TimeStamp now);
    static int64_t numCreated(){
        return s_numCreated_.get();
    }
private:
    const TimerCallback callback_;
    TimeStamp expiration_;
    const double interval_;
    const bool repeat_;
    const int64_t sequence_;
    //用静态的原子型整数
    //使得所有的类都使用
    //并且线程池安全
    static AtomicInt64 s_numCreated_;
};
}
}