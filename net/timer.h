#pragma once
#include"../base/atomic.h"
#include"../base/Timestamp.h"
#include"callbacks.h"
//��ʱ����

//ӵ�лص�����

//��ʱʱ��
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
    //�þ�̬��ԭ��������
    //ʹ�����е��඼ʹ��
    //�����̳߳ذ�ȫ
    static AtomicInt64 s_numCreated_;
};
}
}