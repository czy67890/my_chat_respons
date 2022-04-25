#pragma once
#include"../base/nocopyable.h"
#include"../base/time_stamp.h"
#include<functional>
#include<memory>
namespace czy{
namespace net{
class EventLoop;
class Channel: nocopyable{

private:
    static std::string eventToString(int fd,int ev);
    void update();
    void handleEventWithGuard(TimeStamp receiveTime);
    static const int KNoneEvent;
    static const int KReadEvent;
    static const int KwriteEvent;
    EventLoop * loop_;
    const int fd_;
    int events_;
    int revents_;
    int index_;
    bool logHup_;
    //std::weak_ptr用于与shared_ptr协作管理
    std::weak_ptr<void> tie_;
    ReadEventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;
};
}
}