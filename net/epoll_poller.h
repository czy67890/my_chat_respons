#pragma once
#include"poller.h"
using namespace czy;
using namespace czy::net;
#include<vector>

struct epoll_event;
namespace czy{

namespace net{

class EPollPoller :public Poller{
public:
    EPollPoller(EventLoop * loop);
    //override指示该函数必须被覆写
    //用来保证不发生一些错误
    ~EPollPoller() override;
private:
    static const int KInitEventSize;
    static const char * operationTostring(int op);
    //函数后面的const只是保证不修改//成员变量
    void fillActiveList(int numEvent,ChannelList *activeList) const;
    void update(int op,Channel *channel);
    using EventList = std::vector<struct epoll_event>;
    int epollfd_;
    EventList eventList_;
};
}
}