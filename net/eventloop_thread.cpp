#include "eventloop_thread.h"
#include"event_loop.h"

using namespace czy;
using namespace czy::net;
EventLoopThread::EventLoopThread(const ThreadInitCallback & cb,const string & name)
 :loop_(NULL),exiting_(false),thread_(std::bind(&EventLoopThread::threadFunc,this),name),
 mutex_(),cond_(mutex_),callback_(cb)
{ }

EventLoopThread::~EventLoopThread(){
    exiting_ = true;
    if(loop_ != NULL){
 
        loop_->quit();
        thread_.jion();
    }
}

EventLoop * EventLoopThread::startLoop(){
    assert(!thread_.started());
    thread_.start();
    EventLoop * loop = NULL;
    {
        MutexGroud lock(mutex_);
        while(loop_ == NULL){
            cond_.wait();
        }
        loop = loop_;
    }
    return loop;
}

void EventLoopThread::threadFunc(){
    EventLoop loop;
    if(callback_){
        callback_(&loop);
    }
    {
        MutexGroud lock(mutex_);
        loop_  = &loop;
        cond_.post();
    }
    loop.loop();
    MutexGroud lock(mutex_);
    loop_ = NULL;
}