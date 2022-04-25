#pragma once
#include<atomic>
#include<functional>
#include<vector>
#include<memory>
#include<boost/any.hpp>
#include"../base/locker.h"
#include"../base/current_thread.h"
#include"../base/time_stamp.h"
#include"timer_id.h"
#include"callbacks.h"
#include"../base/nocopyable.h"
namespace czy{

namespace net{
//���ڷַ�����
class Channel;
//���ڽ���Ծ������ӵ�chanel�зַ�
class Poller;
class TimerQueue;
class EventLoop: public nocopyable{
public:
    typedef std::function<void()> Functor;
    EventLoop();
    ~EventLoop();

    void loop();
    void quit();
    void runInLoop(Functor cb);
    void queueInLoop(Functor cb);
    size_t queueSize() const;
    TimerId runAt(TimeStamp time,TimerCallback cb);
    TimerId runAfter(double delay,TimerCallback cb);
    TimerId runEvery(double interval,TimerCallback cb);
    void cancle(TimerId timerid);
    void wakeup();
    void updateChannel(Channel *channel);
    void removeChannel(Channel *channel);
    bool hasChannel(Channel* channel);
    void assertInLoopThread(){
        if(!isInLoopThread()){
            abortNotInLoopThread();
        }
    }
    bool isInLoopThread() const{
        return threadId_ == CurrentThread::tid();
    }
    bool eventHandling() const{return eventHandling_;}
    //boost::any ��һ�ֿ���ʵ���κ����͵�boost��
    //��ԭ���ǻ�����һ��ָ��
    //ָ���������ݣ�����ʹ��ģ��
    void setContext(const boost::any& context){
        context_ = context;
    }
    const boost::any& getContext(){
        return &context_;
    }
    boost::any* getMutableContext(){
        return &context_;
    }
    static EventLoop* getEventLoopOfCurrentThread();
    TimeStamp pollReturnTime() const { return pollReturnTime_;}
    int64_t iteration() const {return iteration_;}
private:
    void abortNotInLoopThread();
    void handleRead();
    void dePendingFunctors();
    void printActiveChannels() const;
    typedef std::vector<Channel *>ChannelList;
    bool looping_;
    std::atomic<bool> quit;
    bool eventHandling_;
    int64_t iteration_;
    const pid_t threadId_;
    TimeStamp pollReturnTime_;
    std::unique_ptr<Poller> poller_;
    std::unique_ptr<TimerQueue> timerQueue_;
    int wakeupFd_;
    std::unique_ptr<Channel> wakeupChannel_;
    boost::any context_;
    ChannelList activeChannels_;
    Channel* currentActiveChannel_;
    mutable Mutex mutex_;
    std::vector<Functor> pendingFunctors_;
};
}
}
