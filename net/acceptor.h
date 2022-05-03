#pragma once
#include<functional>

#include"channel.h"
#include"sockets.h"

namespace czy{
namespace net{

class EventLoop;
class InetAddress;

//Acceptor的主要功能。接受链接
//但是又要使得在文件描述符超载的时候能够继续处理，不至于使得程序永远停等
//方法:首先打开一个空文件，占领一个描述符
//这个描述符是用来占位的
//如果说出现打开文件描述符过多的错误时
//先关闭占位符
//然后链接到这个占位符
//让client认为自己链接成功
//然后立马关闭
//并且让该占位符回到占领该文件描述符上
class Acceptor :nocopyable{
public:
    using NewConnectionCallback = std::function<void(int sockfd,const InetAddress&)>;
    Acceptor(EventLoop * loop,const InetAddress & listenaddr,bool reuse);
    ~Acceptor();
    void setNewConnectionCallback(const NewConnectionCallback& cb);
    void listen();

    bool listening() const { return listening_; }

  // Deprecated, use the correct spelling one above.
  // Leave the wrong spelling here in case one needs to grep it for error messages.
  // bool listenning() const { return listening(); }

private:
    void handleRead();
    EventLoop * loop_;
    Socket acceptSocket_;
    Channel acceptChannel_;
    NewConnectionCallback newConnectionCallback_;
    bool listening_;
    int idleFd_;
};
}
}