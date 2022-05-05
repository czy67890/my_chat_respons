#include "epoll_poller.h"
#include "channel.h"
#include <assert.h>
#include <errno.h>
#include <poll.h>
#include <sys/epoll.h>
#include <unistd.h>
#include "../base/logging.h"
using namespace czy;
using namespace czy::net;

namespace
{
    const int KNew = -1;
    const int KAdded = 1;
    const int KDeleted = 2;
}

EPollPoller::EPollPoller(EventLoop *loop)
    // epoll_create1()参数为EPOLL_CLOEXEC时，为fd设置执行时关闭的选项
    //即当调用exec系统调用时，会关闭相应的描述符
    : Poller(loop), epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
      eventList_(KInitEventSize)
{
    if (epollfd_ < 0)
    {
        LOG_SYSFATAL << "EpollPoller::EPollPoller";
    }
}

EPollPoller::~EPollPoller()
{
    ::close(epollfd_);
}

TimeStamp EPollPoller::poll(int timeoutMs, ChannelList *activeChannel)
{
    LOG_TRACE << "fd total count " << channels_.size();
    int numEvents = ::epoll_wait(epollfd_, &*eventList_.begin(), static_cast<int>(eventList_.size()), timeoutMs);
    //手法，有可能出现新错误的地方，把旧错误存起来先
    int savedErrorno = errno;
    TimeStamp now(TimeStamp::now());
    if (numEvents > 0)
    {
        LOG_TRACE << numEvents << " events happend";
        fillActiveList(numEvents, activeChannel);
        if (implicit_cast<size_t>(numEvents) == eventList_.size())
        {
            eventList_.resize(numEvents << 1);
        }
    }
    else if (numEvents == 0)
    {
        LOG_TRACE << "nothing happend";
    }
    else
    {
        if (savedErrorno != EINTR)
        {
            errno = savedErrorno;
            LOG_SYSERR << "EPOllPOller::poll()";
        }
    }
    return now;
}

void EPollPoller::fillActiveList(int numEvents, ChannelList *activechannel) const
{
    assert(implicit_cast<size_t>(numEvents) <= eventList_.size());
    for (int i = 0; i < numEvents; ++i)
    {
        Channel *channel = static_cast<Channel *>(eventList_[i].data.ptr);
#ifndef NODEBUG
        int fd = channel->fd();
        //在poller之前肯定被加入到map中了

        ChannelMap::const_iterator iter = channels_.find(fd);
        assert(iter != channels_.end());
        assert(channel == iter->second);
#endif
        channel->setRevents(eventList_[i].events);
        activechannel->push_back(channel);
    }
}

void EPollPoller::updateChannel(Channel *channel)
{
    Poller::assertInLoopThread();
    const int index = channel->index();
    LOG_TRACE << "fd = " << channel->fd() << " event " << channel->events() << " index " << index;
    if (index == KNew || index == KDeleted)
    {
        int fd = channel->fd();
        if (index == KNew)
        {
            assert(channels_.find(fd) == channels_.end());
            channels_[fd] = channel;
        }
        else
        {
            assert(channels_.find(fd) != channels_.end());
            assert(channels_[fd] == channel);
        }
        channel->set_index(KAdded);
        update(EPOLL_CTL_ADD, channel);
    }
    else
    {
        int fd = channel->fd();
        (void)fd;
        assert(channels_.find(fd) != channels_.end());
        assert(channels_[fd] = channel);
        assert(index == KAdded);
        if (channel->isNoneEvent())
        {
            channel->set_index(KDeleted);
            update(EPOLL_CTL_DEL, channel);
        }
        else
        {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

void EPollPoller::removeChannel(Channel *channel)
{
    Poller::assertInLoopThread();
    int fd = channel->fd();
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] = channel);
    assert(channel->isNoneEvent());
    int index = channel->index();
    assert(index == KAdded || index == KDeleted);
    size_t n = channels_.erase(fd);
    (void)(n);
    assert(n == 1);
    if (index == KAdded)
    {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->set_index(KNew);
}

void EPollPoller::update(int operation, Channel *channel)
{
    struct epoll_event event;
    mem_zero(&event, sizeof(event));
    // epoll_event的data是一个union
    //我们使用其中的ptr来存放channel的地址
    event.data.ptr = channel;
    event.events = channel->events();
    int fd = channel->fd();
    LOG_TRACE << "epoll_ctl op = " << operationTostring(operation) << "fd = "
              << fd << " event = "
              << "{ " << channel->eventsToString() << " }";
    if (::epoll_ctl(epollfd_, operation, fd, &event) < 0)
    {
        if (operation == EPOLL_CTL_DEL)
        {
            LOG_SYSERR << "epoll_ctl op =" << operationTostring(operation) << " fd =" << fd;
        }
        else
        {
            LOG_SYSFATAL << "epoll_ctl op =" << operationTostring(operation) << " fd =" << fd;
        }
    }
}
const char* EPollPoller::operationTostring(int op)
{
  switch (op)
  {
    case EPOLL_CTL_ADD:
      return "ADD";
    case EPOLL_CTL_DEL:
      return "DEL";
    case EPOLL_CTL_MOD:
      return "MOD";
    default:
      assert(false && "ERROR op");
      return "Unknown Operation";
  }
}
