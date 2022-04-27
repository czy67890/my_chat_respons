#include"timer.h"
using namespace czy;
using namespace czy::net;
AtomicInt64 Timer::s_numCreated_;
void Timer::restart(TimeStamp now){
    if(repeat_){
        expiration_ = add_time(now,interval_);
    }
    else{
        expiration_ = TimeStamp::invalid();
    }
}