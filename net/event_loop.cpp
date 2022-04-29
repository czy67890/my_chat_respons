#include"../net/event_loop.h"
#include"../base/logging.h"
#include"../base/locker.h"
#include"channel.h"
#include"poller.h"
#include"timer_queue.h"
#include"socketops.h"
#include"timer_queue.h"
#include<algorithm>
#include<signal.h>
#include<sys/eventfd.h>
#include<unistd.h>
using namespace czy;
using namespace czy::net;
namespace {
//__thread指示每个线程所特有的变量
__thread EventLoop * t_loopInThisThread = 0;
const int KPollTimeMs = 10000;
int createEventfd(){
    //eventfd用于进程或者线程中的消息共享
    //使用方法创建一个eventfd
    //write会将整数类型加到初始的值上
    //而read方法则会将这个值读出
    //EFD_NONBLOCK使得非阻塞
    //EFD_CLOEXEC在退出后自动释放自己所持有的资源
    int evtfd = eventfd(0,EFD_NONBLOCK|EFD_CLOEXEC);    
    if(evtfd < 0){
        LOG_SYSERR<<"Failed in event fd";
        //abort 生成一个coredump
        abort();
    }
    return evtfd;
}

#pragma GCC diagnostic ignored "-Wold-style-cast"

class IgnoreSigPipe{
public:
    IgnoreSigPipe(){
        //signal函数代表本进程遇到第一个参数指示的信号后
        //会自动调用后一个参数指定的函数
        //第二个参数的特殊值
        //1.SIG_IGN:表示忽略该信号
        //2.SIG_DFL:表示恢复对该信号的默认处理
        ::signal(SIGPIPE,SIG_IGN);
    }
};
IgnoreSigPipe initobj;
}

EventLoop * EventLoop::getEventLoopOfCurrentThread(){
    return t_loopInThisThread;
}

EventLoop::EventLoop()
 :looping_(false),quit_(false),eventHandling_(false),
  callingPendingFunctors_(false),iteration_(0),
  threadId_(CurrentThread::tid()),poller_(Poller::newDefaultPOller(this)),
  timerQueue_(new TimerQueue(this)),wakeupFd_(createEventfd()),
  wakeupChannel_(new Channel(this,wakeupFd_)),currentActiveChannel_(NULL)
{   
    //这里的this会被转换成一个空指针，然后输出

    //LOG_DEBUG这些实质上都是LOG_STREAM类
    LOG_DEBUG<<"EventLoop created"<<this<<"in thread"<<threadId_;
    if(t_loopInThisThread){
        LOG_FATAL<<"Another EventLoop "<<t_loopInThisThread
                 <<"exists in this thread "<<threadId_;
    }
    else{
        t_loopInThisThread = this;       
    }
    wakeupChannel_->setReadCallback(
        std::bind(&EventLoop::handleRead,this));
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop(){
    LOG_DEBUG<<"EventLoop "<<this<<"of thread"<<threadId_
             <<"desttcus in thread "<<CurrentThread::tid();
    wakeupChannel_->disAbleAll();
    wakeupChannel_->remove();
    ::close(wakeupFd_);
    t_loopInThisThread = NULL;
}

//EventLoop的事件主循环

void EventLoop::loop(){
    assert(!looping_);
    assertInLoopThread();
    looping_ = false;
    quit_ = true;
    LOG_TRACE<<"EventLoop "<<this<<" start looping";
    while(!quit_){
        activeChannels_.clear();
        //poller->poll()需要返回一个TimeStamp
        //并且将活跃的时间写入到activeChannels中
        pollReturnTime_ = poller_->poll(KPollTimeMs,&activeChannels_);
        ++iteration_;
        if(Logger::log_level() <= Logger::TRACE){
            printActiveChannels();
        }
        eventHandling_ = true;
        for(Channel * channel : activeChannels_){
            currentActiveChannel_ = channel;
            currentActiveChannel_->handleEvent(pollReturnTime_);
        }
        currentActiveChannel_ = NULL;
        eventHandling_ = false;
        doPendingFunctors();
    }

    LOG_TRACE<<" EventLoop "<<this<<" stop looping";
    looping_ = false;
}

void EventLoop::quit(){
    quit_ = true;
    if(!isInLoopThread()){
        wakeup();
    }
}

//在线程中运行回调函数
//若是当前IO线程
//直接调用回调函数
//若不是则需要插入到队列中
//等待调用
void EventLoop::runInLoop(Functor cb){
    if(isInLoopThread){
        cb();
    }
    else{
        queueInLoop(std::move(cb));
    }
}

//typedef 的魅力
void EventLoop::queueInLoop(Functor cb){
    //花括号手动管理变量周期
    //减少临界区长度
    //释放无用内存
    {
        MutexGroud lock(mutex_);
        pendingFunctors_.push_back(std::move(cb));
    }
    if(!isInLoopThread() || callingPendingFunctors_){
        wakeup();
    }
}

size_t EventLoop::queueSize() const{
    MutexGroud lock(mutex_);
    return pendingFunctors_.size(); 
}


//EventLoop中的三个运行函数
//RunAt直接插入队列运行
//RunAfter则在设置delay后运行
//RunEvery则插入队列，之后设置interval后运行
TimerId EventLoop::runAt(TimeStamp time,TimerCallback cb){
    return timerQueue_->addTimer(std::move(cb),time,0.0);
}

TimerId EventLoop::runAfter(double delay,TimerCallback cb){
    TimeStamp time(add_time(TimeStamp::now() ,delay));
    return runAt(time,std::move(cb));
}

TimerId EventLoop::runEvery(double interval,TimerCallback cb){
    TimeStamp time(add_time(TimeStamp::now(),interval));
    return timerQueue_->addTimer(std::move(cb),time,interval);
}

void EventLoop::cancel(TimerId timerid){
    return timerQueue_->cancel(timerid);
}

void EventLoop::updateChannel(Channel * channel){
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    //将updateChannel 交给poller去处理
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel *channel){
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    if(eventHandling_){
        //若正在处理事件
        //则需要当前的活动channel为要关闭的channel
        //或者该channel并不在活动channel中
        assert(currentActiveChannel_ == channel ||
        std::find(activeChannels_.begin(),activeChannels_.end(),channel) == activeChannels_.end());
    }
    poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel *channel){
    assert(channel->ownerLoop() == this);
    assertInLoopThread();;
    return poller_->hasChannel(channel);
}

void EventLoop::abortNotInLoopThread(){
    LOG_FATAL<<"EventLoop::abortNotInLoop Thread - EventLoop"<<
    this<<" was created in threadId_ = "<< threadId_<<", current thread id = "<< CurrentThread::tid();
}

void EventLoop::wakeup(){
    uint64_t one = 1;
    //wakeup仅仅只是写一个字节到wakeupFd_中
    //达到唤醒io线程的目的
    ssize_t n = sockets::write(wakeupFd_,static_cast<const char *>((void*) &one),sizeof(one)); 
    if(n != sizeof(one)){
        LOG_ERROR<<"EventLoop::wakeup() writes "<<n<<" bytes instead of 8 bytes";
    }
}

void EventLoop::handleRead(){
    uint64_t one = 1;
    ssize_t n = sockets::read(wakeupFd_,&one,sizeof(one));
    if(n != sizeof(one)){
        LOG_ERROR<<"EventLoop::handleRead() writes "<<n<<" bytes instead of 8 bytes";
    } 
}

void EventLoop::doPendingFunctors(){
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;
    //依然是减少临界区
    {
        MutexGroud lock(mutex_);
        std::swap(functors,pendingFunctors_);
    }
    for(const Functor &cb:functors){
        cb();
    }
    callingPendingFunctors_ = false;
}

void EventLoop::printActiveChannels() const{
    for(const Channel * channel:activeChannels_){
        LOG_TRACE<<"{"<<channel->reventsToString()<<"}";
    }
}