#pragma once
#include"../base/nocopyable.h"
#include"inetaddress.h"
#include<functional>
#include<memory>

namespace czy{
namespace net{

class Channel;

class EventLoop;

//enable_shared_from_this 允许使用shared_ptr安全的管理对象的this指针
class Connector:nocopyable
                , public std::enable_shared_from_this<Connector>
{
public:
    using NewConnectionCallback = std::function<void(int sockfd)>;
    Connector(EventLoop * loop,const InetAddress & serverAddr);
    ~Connector();

    void setNewConnectionCallback(const NewConnectionCallback& cb){
        newConnectionCallback_ = cb;
    }

    void start();
    void restart();
    void stop();
    const InetAddress & serverAddress() const {return serverAddr_;}
private:
    enum States{KDisconnected,KConnecting,KConnected};
    //默认的最大重试延迟30s
    static const int KMaxRetryDelayMs = 30*1000;
    //默认初始的重试时延0.5s
    static const int KInitRetryDelayMs = 500;
    void setState(States s){state_ = s;}
    void startInLoop();
    void stopInLoop();
    void connect();
    void connecting(int sockfd);
    void handleWrite();
    void handleEror();
    void retry(int sockfd);
    int removeAndReserChannel();
    void resetChannel();
    NewConnectionCallback newConnectionCallback_;
    EventLoop *loop_;
    InetAddress serverAddr_;
    bool connect_;
    States state_;
    std::unique_ptr<Channel> channel_;
    int retryDelayMs_;
};
}
}