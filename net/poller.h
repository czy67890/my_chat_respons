#pragma once
#include<map>
#include<vector>
#include"../base/time_stamp.h"
#include"event_loop.h"
namespace czy{

namespace net{

class Channel;

class Poller : nocopyable{
public:
    typedef std::vector<Channel *> ChannelList;
    Poller(EventLoop * loop);
    //基类的析构函数必须设置成虚函数
    virtual ~Poller();

    virtual TimeStamp poll(int timeoutMs,ChannelList *activeChannel) = 0;

    //纯虚函数必须被继承类实现
    //一般与override搭配
    //有效的防止写错的情况
    virtual void updateChannel(Channel * channel) = 0;

    virtual void removeChannel(Channel *channel) = 0;

    virtual bool hasChannel(Channel * channel) const;
    static Poller* newDefaultPOller(EventLoop * loop);
    void assertInLoopThread() const{
        ownerLoop_->assertInLoopThread();
    } 
//设置成protected属性后
//可以被派生类访问
//private不能被派生类访问
protected:
    typedef std::map<int,Channel *> ChannelMap;
    ChannelMap channels_;
private:
    EventLoop* ownerLoop_;

};
}
}
