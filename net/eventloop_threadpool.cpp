#include"eventloop_threadpool.h"
#include"event_loop.h"
#include"eventloop_thread.h"
#include<stdio.h>

using namespace czy;
using namespace czy::net;

EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseloop,const string & name)
 :baseloop_(baseloop),name_(name),started_(false),numThreads_(0),next_(0)
{
}

//手动管理析构函数
//不删除loop
EventLoopThreadPool::~EventLoopThreadPool(){
    //dont delete loop ,it's stack varable
}

void EventLoopThreadPool::start(const ThreadInitCallback &cb){
    assert(!started_);
    baseloop_->assertInLoopThread();
    started_ = true;
    for(int i = 0;i<numThreads_ ;++i){
        char buf[name_.size() + 32];
        snprintf(buf,sizeof(buf),"%s%d",name_.c_str(),i);
        EventLoopThread *t = new EventLoopThread(cb,buf);
        threads_.push_back(std::unique_ptr<EventLoopThread>(t));
        loops_.push_back(t->startLoop());
    }
    if(numThreads_ == 0&& cb){
        cb(baseloop_);
    }
}

EventLoop * EventLoopThreadPool::getNextLoop(){
    baseloop_ ->assertInLoopThread();
    assert(started_);
    EventLoop * loop = baseloop_;
    if(!loops_.empty()){
        loop = loops_[next_];
        next_++;
        if(next_ == static_cast<int>(loops_.size())){
            next_ = 0;
        }
    }
    return loop;
}

EventLoop * EventLoopThreadPool::getLoopForHash(size_t hashCode){
    baseloop_->assertInLoopThread();
    assert(started_);
    if(!loops_.empty()){
        return loops_[hashCode%loops_.size()]; 
    }
    return NULL;
}

std::vector<EventLoop*> EventLoopThreadPool::getAllLoops(){
    baseloop_->assertInLoopThread();
    assert(started_);
    if(loops_.empty()){
        return std::vector<EventLoop*> (1,baseloop_);
    }
    return loops_;
}
