#pragma once 
#include"../base/nocopyable.h"
#include"../base/types.h"
#include<functional>
#include<memory>
#include<vector>

namespace czy{
namespace net{

class EventLoop;
class EventLoopThread;
class EventLoopThreadPool :nocopyable{
public:

    typedef std::function<void (EventLoop*)> ThreadInitCallback;
    EventLoopThreadPool(EventLoop * baseloop,const string & nameArg);
    ~EventLoopThreadPool();
    void setThreadNum(int numThreads) {numThreads_ = numThreads;}
    void start(const ThreadInitCallback &cb = ThreadInitCallback());

    EventLoop * getNextLoop();
    EventLoop * getLoopForHash(size_t hashCode);

    std::vector<EventLoop*> getAllLoops();
    bool started() const{ return started_; }

    const string &name ()const{
        return name_;
    }

private:
    EventLoop * baseloop_;
    string name_;
    bool started_;
    int numThreads_;
    int next_;
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop*> loops_;
};
}
}