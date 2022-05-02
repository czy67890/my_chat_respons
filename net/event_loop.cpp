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
//__threadָʾÿ���߳������еı���
__thread EventLoop * t_loopInThisThread = 0;
const int KPollTimeMs = 10000;
int createEventfd(){
    //eventfd���ڽ��̻����߳��е���Ϣ����
    //ʹ�÷�������һ��eventfd
    //write�Ὣ�������ͼӵ���ʼ��ֵ��
    //��read������Ὣ���ֵ����
    //EFD_NONBLOCKʹ�÷�����
    //EFD_CLOEXEC���˳����Զ��ͷ��Լ������е���Դ
    int evtfd = eventfd(0,EFD_NONBLOCK|EFD_CLOEXEC);    
    if(evtfd < 0){
        LOG_SYSERR<<"Failed in event fd";
        //abort ����һ��coredump
        abort();
    }
    return evtfd;
}

#pragma GCC diagnostic ignored "-Wold-style-cast"

class IgnoreSigPipe{
public:
    IgnoreSigPipe(){
        //signal������������������һ������ָʾ���źź�
        //���Զ����ú�һ������ָ���ĺ���
        //�ڶ�������������ֵ
        //1.SIG_IGN:��ʾ���Ը��ź�
        //2.SIG_DFL:��ʾ�ָ��Ը��źŵ�Ĭ�ϴ���
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
    //�����this�ᱻת����һ��ָ�룬Ȼ�������ֵַ

    //LOG_DEBUG��Щʵ���϶���LOG_STREAM��
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

//EventLoop���¼���ѭ��

void EventLoop::loop(){
    assert(!looping_);
    assertInLoopThread();
    looping_ = false;
    quit_ = true;
    LOG_TRACE<<"EventLoop "<<this<<" start looping";
    while(!quit_){
        activeChannels_.clear();
        //poller->poll()��Ҫ����һ��TimeStamp
        //���ҽ���Ծ��ʱ��д�뵽activeChannels��
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

//���߳������лص�����
//���ǵ�ǰIO�߳�
//ֱ�ӵ��ûص�����
//����������Ҫ���뵽������
//�ȴ�����
//channel��ֻ��Ҫ����
//bind����
//���ص�����bind����runInLoop�м���

void EventLoop::runInLoop(Functor cb){
    if(isInLoopThread){
        cb();
    }
    else{
        queueInLoop(std::move(cb));
    }
}

//typedef ������
void EventLoop::queueInLoop(Functor cb){
    //�������ֶ������������
    //�����ٽ�������
    //�ͷ������ڴ�
    {
        MutexGroud lock(mutex_);
        pendingFunctors_.push_back(std::move(cb));
    }
    if(!isInLoopThread() || callingPendingFunctors_){
        //wakeupֱ��send������ȡio�̵߳Ŀ���Ȩ
        wakeup();
    }
}

size_t EventLoop::queueSize() const{
    MutexGroud lock(mutex_);
    return pendingFunctors_.size(); 
}


//EventLoop�е��������к���
//RunAtֱ�Ӳ����������
//RunAfter��������delay������
//RunEvery�������У�֮������interval������
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
    //��updateChannel ����pollerȥ����
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel *channel){
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    if(eventHandling_){
        //�����ڴ����¼�
        //����Ҫ��ǰ�ĻchannelΪҪ�رյ�channel
        //���߸�channel�����ڻchannel��
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
    //wakeup����ֻ��дһ���ֽڵ�wakeupFd_��
    //�ﵽ����io�̵߳�Ŀ��
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
    //��Ȼ�Ǽ����ٽ���
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

