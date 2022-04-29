#include"poller.h"
#include"channel.h"

using namespace czy;
using namespace czy::net;

Poller::Poller(EventLoop *loop)
 :ownerLoop_(loop)
{ }

Poller::~Poller() = default;

bool Poller::hasChannel(Channel *channel) const{
    assertInLoopThread();
    ChannelMap::const_iterator iter = channels_.find(channel->fd());
    return iter != channels_.end() && iter->second == channel; 
}