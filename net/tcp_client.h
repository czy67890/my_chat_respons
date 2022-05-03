#pragma once
#include"../base/locker.h"
#include"tcp_connection.h"
namespace czy{
namespace net{

class Connector;
using ConnectorPtr = std::shared_ptr<Connector>;

class TcpClient : nocopyable{
public:
    TcpClient(EventLoop * loop,const InetAddress & serverAddr,
    const string & nameArg);
    ~TcpClient();
    void connect();
    void disconnect();
    void stop();
    TcpConnectionPtr connection() const{
        MutexGroud lock(mutex_);
        return connection_;
    }

    EventLoop * getLoop()const{
        return loop_;
    }

    bool retry() const{ return retry_;}
    void enableretry(){ retry_ = true;}
    const string & name() const{
        return name_;
    }

    void setConnectionCallback(ConnectionCallback cb){
        //使用move来提高效率
        connectionCallback_ = std::move(cb);
    }

    void setMessageCallback(MessageCallback cb){
        messageCallback_ = std::move(cb);
    }

    void setWriteCompleteCallback(WriteCompleteCallback cb){
        writeCompleteCallback_ = std::move(cb);
    }

    
private:
    void newConnection(int sockfd);
    void removeConnection(const TcpConnectionPtr & conn);

    EventLoop * loop_;
    ConnectorPtr connector_;
    const string name_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    bool retry_;
    bool connect_;
    int nextConnId_;
    //Mutex前面必须加上mutable
    mutable Mutex mutex_;
    TcpConnectionPtr connection_;
};
}
}
