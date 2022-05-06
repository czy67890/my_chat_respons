#pragma once

#include"../base/atomic.h"
#include"../base/types.h"
#include"tcp_connection.h"

#include<map>

namespace czy{
namespace net{

class Acceptor;
class EventLoop;
class EventLoopThreadPool;

class TcpServer :nocopyable{
public:
    //����д�����typedef����Ҫ��
    using ThreadInitCallback = std::function<void(EventLoop*)> ;
    enum Option{
        KNoReusePort,
        kReusePort,
    };
    TcpServer(EventLoop *loop,const InetAddress & listenAddr,const string & nameArg,Option option = KNoReusePort);
    ~TcpServer();
    const string& inPort() const {return ipPort_;}
    const string& name() const {return name_;}
    EventLoop* getLoop() const {return loop_;}
    void setThreadNum(int numThreads);
  void setThreadInitCallback(const ThreadInitCallback& cb)
  { threadInitCallback_ = cb; }
  /// valid after calling start()
  std::shared_ptr<EventLoopThreadPool> threadPool()
  { return threadPool_; }

  /// Starts the server if it's not listening.
  ///
  /// It's harmless to call it multiple times.
  /// Thread safe.
  void start();

  /// Set connection callback.
  /// Not thread safe.
  void setConnectionCallback(const ConnectionCallback& cb)
  { connectionCallback_ = cb; }

  /// Set message callback.
  /// Not thread safe.
  void setMessageCallback(const MessageCallback& cb)
  { messageCallback_ = cb; }

  /// Set write complete callback.
  /// Not thread safe.
  void setWriteCompleteCallback(const WriteCompleteCallback& cb)
  { writeCompleteCallback_ = cb; }

private:
    void newConnection(int sockfd,const InetAddress &peerAddr);
    void removeConnection(const TcpConnectionPtr & conn);
    void removeConnectionInLoop(const TcpConnectionPtr &conn);
    //TcpServer�ĺ������ݽṹ
    //���name��Ӧ��TcpConnection
    using ConnectionMap = std::map<string,TcpConnectionPtr>;


    //�����������ĸ�loop
    //��reactor���ڵ�loop
    
    EventLoop *loop_;
    const string ipPort_;
    const string name_;
    std::unique_ptr<Acceptor> acceptor_;
    std::shared_ptr<EventLoopThreadPool> threadPool_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    ThreadInitCallback threadInitCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    AtomicInt32 started_;
    int nextConnId_;
    ConnectionMap connections_;
};
}
}