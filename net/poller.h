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
    //��������������������ó��麯��
    virtual ~Poller();

    virtual TimeStamp poll(int timeoutMs,ChannelList *activeChannel) = 0;

    //���麯�����뱻�̳���ʵ��
    //һ����override����
    //��Ч�ķ�ֹд������
    virtual void updateChannel(Channel * channel) = 0;

    virtual void removeChannel(Channel *channel) = 0;

    virtual bool hasChannel(Channel * channel) const;
    static Poller* newDefaultPOller(EventLoop * loop);
    void assertInLoopThread() const{
        ownerLoop_->assertInLoopThread();
    } 
//���ó�protected���Ժ�
//���Ա����������
//private���ܱ����������
protected:

    //pollerӵ��fd��Channel*��ӳ��
    typedef std::map<int,Channel *> ChannelMap;
    ChannelMap channels_;
private:
    EventLoop* ownerLoop_;

};
}
}
