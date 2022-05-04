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
    //overrideָʾ�ú������뱻��д
    //������֤������һЩ����
    ~EPollPoller() override;
private:
    static const int KInitEventSize;
    static const char * operationTostring(int op);
    //���������constֻ�Ǳ�֤���޸�//��Ա����
    void fillActiveList(int numEvent,ChannelList *activeList) const;
    void update(int op,Channel *channel);
    using EventList = std::vector<struct epoll_event>;
    int epollfd_;
    EventList eventList_;
};
}
}