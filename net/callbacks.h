#pragma once
#include<functional>
#include"../base/time_stamp.h"
#include<memory>
#include"../base/types.h"
namespace czy{
//用于bind中的占位符
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
template<typename T>

inline T *get_pointer(const std::shared_ptr<T>& ptr){
    return ptr.get();
}
template<typename T>
inline T *get_pointer(const std::unique_ptr<T> & ptr){
    return ptr.get();
}
template<typename To,typename From>
inline ::std::shared_ptr<To> down_pointer_cast(const ::std::shared_ptr<From>& ptr){
    if(false){
        implicit_cast<From*,To*>(0)
    }
    return ::std::static_pointer_cast<To>(f);
}
namespace net{

class Buffer;
class TcpConnection;
typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
typedef std::function<void()> TimerCallback;
typedef std::function<void (const TcpConnectionPtr&)> ConnectionCallback;
typedef std::function<void (const TcpConnectionPtr&)> CloseCallback;
typedef std::function<void (const TcpConnectionPtr&)> WriteCompleteCallback;
typedef std::function<void (const TcpConnectionPtr&, size_t)> HighWaterMarkCallback;

typedef std::function<void (const TcpConnectionPtr&,
                            Buffer*,
                            TimeStamp)> MessageCallback;

void defaultConnectionCallback(const TcpConnectionPtr& conn);
void defaultMessageCallback(const TcpConnectionPtr& conn,
                            Buffer* buffer,
                            TimeStamp receiveTime);
}
}
