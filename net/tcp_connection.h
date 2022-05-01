#pragma once
#include<memory>
#include<boost/any.hpp>

#include"../base/nocopyable.h"
#include"../base/string_piece.h"
#include"../base/types.h"
#include"callbacks.h"
#include"buffer.h"
#include"inetaddress.h"
struct tcp_info;
namespace czy{

namespace net{

class Channel;
class EventLoop;
class Socket;


//enable_shared_from_this 使得可以用shared_ptr管理某个对象的this指针
//而不会报错
//否则的话，每个shared_ptr都会认为自己独享这个对象
//造成错误
class TcpConnection : nocopyable,public std::enable_shared_from_this<TcpConnection>{

public:
TcpConnection(EventLoop * loop,const string &name,int sockfd,const InetAddress &localaddr,const InetAddress & peeraddr);
~TcpConnection();
EventLoop * getLoop() const{ return loop_;}
const string &name() const{return name_;}
const InetAddress & localAddress(){return localAddress_;}
const InetAddress & peerAddress() {return peerAddress_;}
bool connected () const{ return state_ == KConnected;}
bool disconnected() const {return state_ == KDisconnected;}
bool getTcpInfo(tcp_info *) const;
string getTcpInfoString() const;

void send(const void *message,int len);
void send(const StringPiece & message);
void send(Buffer* message);
void shutdown();
void forceClose();
void forceCloseWithDealy(double seconds);
void setTcpNoDelay(bool on);
void startRead();
void stopRead();
bool isReading() const {return reading_;}

void sendContext(const boost::any& context){
    context_ = context;
} 

const boost::any& getContext() const{
    return context_;
}

const boost::any* getMutableContext(){
    return &context_;
}

void setConnectionCallback(const ConnectionCallback &cb){
    connectionCallback_ = cb;
}

void setMessageCallback(const MessageCallback &cb){
    messageCallback_ = cb;
}

void setWriteCompleteCallback(const WriteCompleteCallback &cb){
    writeCompleteCallback_ = cb;
}

void setHighMarkCallback(const HighWaterMarkCallback &cb){
    highWaterMarkCallback_ = cb;
}

Buffer* inputBuffer(){
    return &inputBuffer_;
}

Buffer* outputBuffer(){
    return &outputBuffer_;
}

void setCloseCallback(const CloseCallback& cb){
    closeCallback_ = cb;
}

void connectEstablished();
void connectDestory();
private:
    enum StateE{KDisconnected,KConnecting,KConnected,KDisConnecting};
    //变成指针，防止报不允许使用不完整类型的错误
    void handleRead(TimeStamp recvtime);
    void handleWrite();
    void handleClose();
    void handleError();
    void sendInLoop(const StringPiece & message);
    void sendInLoop(const void* message,size_t len);
    void shotdownInLoop();
    void forcecloseInLoop();
    void setState(StateE state){ state_ = state; }
    const char *stateTostring() const;
    void startReadInLoop();
    void stopReadInLoop();
    EventLoop* loop_;
    const string name_;
    StateE state_;
    bool reading_;
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;
    const InetAddress localAddress_;
    const InetAddress peerAddress_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    HighWaterMarkCallback highWaterMarkCallback_;
    CloseCallback closeCallback_;
    size_t highWaterMark_;
    Buffer inputBuffer_;
    Buffer outputBuffer_;
    boost::any context_;
};
}
}
