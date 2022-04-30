#pragma once
#include"../base/locker.h"
#include"../base/m_thread.h"

namespace czy{

namespace net{

class EventLoop;
class EventLoopThread : nocopyable{

public:
    typedef std::function<void(EventLoop *)> ThreadInitCallback;
    EventLoopThread(const ThreadInitCallback &cb = ThreadInitCallback(),const string & name = string());
    EventLoopThread();
    ~EventLoopThread();
    EventLoop * startLoop();
private:
    void threadFunc();
    EventLoop * loop_ ;
    bool exiting_;
    MThread thread_;
    Mutex mutex_;
    Cond cond_;
    ThreadInitCallback callback_;
};
}
}