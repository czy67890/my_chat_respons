#pragma once
#include<stdint.h>
namespace czy{
namespace net{
class Timer;
class TimerId{
public:
    TimerId(): timer_(nullptr),sequence_(0){
    }
    TimerId(Timer *timer,int64_t sequence):
     timer_(timer),sequence_(sequence)
    {}
    friend class TimerQueue;
private:
    Timer* timer_;
    int64_t sequence_;
};
}
}