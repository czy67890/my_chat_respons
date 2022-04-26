#pragma once
#include"../base/nocopyable.h"
#include"../base/time_stamp.h"
#include<functional>
#include<memory>
namespace czy{
namespace net{
class EventLoop;
class Channel: nocopyable{
typedef std::function<void()> EventCallback;
typedef std::function<void(TimeStamp)> ReadEventCallback;

Channel(EventLoop *loop,int fd);
~Channel();
void handleEvent(TimeStamp recv_time);
void setReadCallback(ReadEventCallback cb){
    //使用move提高效率
    //前提:被调用的类中有正确的move构造函数
    readCallback_ = std::move(cb);
}
void setWriteCallback(EventCallback cb){
    writeCallback_ = std::move(cb);
}
void setCloseCallback(EventCallback cb){
    closeCallback_ =std::move(cb);
}
void setErrorCallback(EventCallback cb){
    errorCallback_ = std::move(cb);
}
//绑定调用handleEvent的EventLoop防止被destory
void tie(const std::shared_ptr<void > &);

int fd() const {return fd_;}
int events() const{return events_;}
void setRevents(int revt) {revents_ = revt;} 
bool isNoneEvent() const {return revents_ == KNoneEvent;}
void enableReading(){ events_ |= KReadEvent;update();}
//通过位运算来设置是否可读
void disableReading(){events_ &= ~KReadEvent;update();}
void enableWriting(){ events_ |= KWriteEvent;update();}
//通过位运算来设置是否可读
void disableWriting(){events_ &= ~KWriteEvent;update();}

bool isWriting () const{return KReadEvent & events_;}
bool isReading() const{return KWriteEvent & events_;}
int index() const { return index_;}
void set_index(int index){ index_ = index;}
std::string reventsToString() const;
std::string eventsToString() const;
void doNotLogUp(){logHup_ = false;}
EventLoop * ownerLoop() {return loop_;}
void remove();
private:
    static std::string eventToString(int fd,int ev);
    void update();
    void handleEventWithGuard(TimeStamp receiveTime);
    static const int KNoneEvent;
    static const int KReadEvent;
    static const int KWriteEvent;
    EventLoop * loop_;
    const int fd_;
    int events_;
    int revents_;
    int index_;
    bool tied_;
    bool logHup_;
    bool eventHandling_;
    bool addToLoop_;
    //std::weak_ptr用于与shared_ptr协作管理
    std::weak_ptr<void> tie_;
    ReadEventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;
};
}
}