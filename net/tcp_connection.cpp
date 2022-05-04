#include"tcp_connection.h"
#include"../base/weak_callback.h"
#include"../base/logging.h"
#include"channel.h"
#include"event_loop.h"
#include"sockets.h"
#include"socketops.h"
#include<errno.h>

using namespace czy;
using namespace czy::net;

void czy::net::defaultConnectionCallback(const TcpConnectionPtr& conn ){
     LOG_TRACE << conn->localAddress().toIpPort() << " -> "
            << conn->peerAddress().toIpPort() << " is "
            << (conn->connected() ? "UP" : "DOWN");
  // do not call conn->forceClose(), because some users want to register message callback only.
}


void czy::net::defaultMessageCallback(const TcpConnectionPtr &conn,Buffer * buf,TimeStamp){
    buf->retriveAll();
}

TcpConnection::TcpConnection(EventLoop *loop
                            ,const string &nameArg,
                            int sockfd,
                            const InetAddress & locadAddress,
                            const InetAddress & peerAddress)
:loop_(loop),name_(nameArg),state_(KConnecting),
reading_(true),socket_(new Socket(sockfd)),channel_(new Channel(loop,sockfd)),
localAddress_(locadAddress),peerAddress_(peerAddress),highWaterMark_(64*1024*1024)                            
{   //使用std::bind()确保一定是由本对象来调用该回调函数
    //tcpconnection的作用就是建立链接
    //以及作为channel的分配实体去执行任务
    channel_->setCloseCallback(std::bind(&TcpConnection::handleClose,this));
    channel_->setErrorCallback(std::bind(&TcpConnection::handleError,this));
    channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite,this));
    channel_->setReadCallback(std::bind(&TcpConnection::handleRead,this,_1));
    LOG_DEBUG<<"TcpConnection::ctor["<<name_<<"] at"<<this<<" fd = "<<sockfd;
    socket_->set_keep_alive(true);    
}

TcpConnection::~TcpConnection()
{
  LOG_DEBUG << "TcpConnection::dtor[" <<  name_ << "] at " << this
            << " fd=" << channel_->fd()
            << " state=" << stateTostring();
  assert(state_ == KDisconnected);
}

bool TcpConnection::getTcpInfo(struct tcp_info* info) const{
    return socket_->get_tcp_info(info);
}

//只有类的成员函数才可以在申明后加const，代表不会对类的成员变量进行修改
string TcpConnection::getTcpInfoString(struct tcp_info *info) const{
  char buf[1024];
  buf[0] = '\0';
  socket_->get_tcp_info_string(buf,sizeof(buf));
  return buf;
}

void TcpConnection::send(const void * data,int len){
  send(StringPiece(static_cast<const char *> (data),len));
}

void TcpConnection::send(const StringPiece & message){
  if(state_ == KConnected){
    if(loop_ ->isInLoopThread()){
      sendInLoop(message);
    }
    else{
      void (TcpConnection::*fp)(const StringPiece& message) =  &TcpConnection::sendInLoop;
      //将其直接用move即可
      loop_->runInLoop(std::bind(fp,this,std::move(message.as_string())));
    }
  }
}

void TcpConnection::send(Buffer *buf){
  //如果当前event_loop就是本线程
  //则直接发送
  //否则先获取到本线程
  //再发送
  if(state_ == KConnected){
    if(loop_->isInLoopThread()){
      sendInLoop(buf->peek(),buf->readableBytes());
      buf->retriveAll();
    }
    else{
      //若非本线程
      //那么先唤醒本线程，再调用sendInLoop
      //注意这些都是用bind函数链接起来的
      void(TcpConnection::*fp) (const StringPiece &message)= &TcpConnection::sendInLoop;
      loop_->runInLoop(std::bind(fp,this,buf->retriveAllAsString()));
    }
  }
}

void TcpConnection::sendInLoop(const void* message,size_t len){
    loop_->assertInLoopThread();
    ssize_t nwrote = 0;
    size_t remaining = len;
    bool faultError = false;
    if(state_ == KDisconnected){
      LOG_WARN<<"disconnected,give up writing";
      return;
    }
    if(!channel_ ->isWriting() && outputBuffer_.readableBytes() == 0){
      nwrote = sockets::write(channel_->fd(),message,len);
      if(nwrote >= 0){
        remaining = len - nwrote;
        if(remaining == 0 && writeCompleteCallback_){
          loop_->queueInLoop(std::bind(writeCompleteCallback_,shared_from_this()));
        }
      }
      else{
        nwrote = 0;
        //EWOULDBLOCK 实质上等于EAGAIN
        //
        if(errno != EWOULDBLOCK){
          LOG_SYSERR<<"TcpConnection:sendInLoop";
          //EPIPE同SIG_PIPE一致
          //产生于四次挥手时，若客户端已经关闭了链接
          //而serve还在发送
          //此时会产生EPIPE
          //ECONNRESET则产生于链接某一方崩溃，需要重新链接的情况
          if(errno == EPIPE || errno == ECONNRESET){
            faultError = true;
          }
        }
      }
    }
  
assert(remaining <= len);
if(!faultError && remaining > 0 ){
  size_t oldLen = outputBuffer_.readableBytes();
  if(oldLen + remaining >= highWaterMark_&&oldLen <highWaterMark_&&highWaterMarkCallback_){
    loop_->queueInLoop(std::bind(highWaterMarkCallback_,shared_from_this(),oldLen+remaining));
  }
  outputBuffer_.append(static_cast<const char *>(message)+ nwrote,remaining);
  if(!channel_->isWriting()){
    channel_->enableWriting();
  }
}
}

void TcpConnection::shutdownInLoop(){
  loop_->assertInLoopThread();
  if(!channel_->isWriting()){
    socket_->shutdown_write();
  }
}

void TcpConnection::shutdown(){
  if(state_ == KConnected){
    setState(KDisConnecting);
    loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop,shared_from_this()));
  }
}

void TcpConnection::forceClose(){
  if(state_ == KConnected || state_ == KDisConnecting){
    setState(KDisConnecting);
    //一个原则，若TcpConnection对象需要交给其他的线程去处理
    //这个时候我们不应该使用this指针去bind而是使用shared_from_this去bind
    //这样才能保证生命周期不会早于函数执行完之前结束
    loop_->queueInLoop(std::bind(&TcpConnection::forceCloseInLoop,shared_from_this()));
  }
}

void TcpConnection::forceCloseWithDealy(double sec){
  if(state_ == KConnected || state_ == KDisConnecting){
    setState(KDisConnecting);
    loop_->runAfter(sec,makeWeakCallback(shared_from_this(),&TcpConnection::forceClose));
  }
}

void TcpConnection::forceCloseInLoop(){
  loop_->assertInLoopThread();
  if(state_ == KConnected || state_ == KDisConnecting){
    handleClose();
  }
}

const char * TcpConnection:: stateTostring() const{
  switch(state_){
    case KDisconnected:
      return "kDisconnected";
    case  KConnecting:
      return "kConnecting";
    case KConnected:
      return "kConnected";
    case KDisConnecting:
      return "kDisconnecting";
    default:
      return "unknown state";
  }
}

void TcpConnection::setTcpNoDelay(bool on){
  socket_->set_tcp_nodelay(on);
}

void TcpConnection::startRead(){
  loop_->runInLoop(std::bind(&TcpConnection::startRead,this));
}

void TcpConnection::startReadInLoop(){
  loop_->assertInLoopThread();
  if(!reading_ || !channel_->isReading()){
    channel_->enableReading();
    reading_ = true;
  }
}

void TcpConnection::stopRead(){
  loop_->runInLoop(std::bind(&TcpConnection::stopReadInLoop,this));
}

void TcpConnection::stopReadInLoop(){
  loop_->assertInLoopThread();
  if(reading_ || channel_->isReading()){
    channel_->disableReading();
    reading_ - true;
  }
}

void TcpConnection::connectEstablished(){
  loop_->assertInLoopThread();
  assert(state_ = KConnecting);
  setState(KConnected);
  //channel 的tie在创建链接时调用
  //传入的数据是shared_from_this
  //智能指针用来管理生命周期的案列
  channel_->tie(shared_from_this());
}

void TcpConnection::connectDestory(){
  loop_->assertInLoopThread();
  if(state_ == KConnected){
    setState(KDisconnected);
    channel_->disAbleAll();
    connectionCallback_(shared_from_this());
  }
  channel_->remove();
}

void TcpConnection::handleRead(TimeStamp rectime){
  loop_->assertInLoopThread();
  int savedErrno = 0;
  ssize_t n = inputBuffer_.readFd(channel_->fd(),&savedErrno);
  if( n > 0){
    messageCallback_(shared_from_this(),&inputBuffer_,rectime);
  }
  else if(n == 0){
    //当n == 0代表对方已发送完
    //这个时候关闭链接
    handleClose();
  }
  else{
    errno = savedErrno;
    LOG_SYSERR<<"TcpConnection::handleRead";
    handleError();
  }
}

void TcpConnection::handleWrite(){
  loop_->assertInLoopThread();
  //这里直接调用sockets的write函数
  if(channel_->isWriting()){
    ssize_t n = sockets::write(channel_->fd(),outputBuffer_.peek(),outputBuffer_.readableBytes());
    if(n > 0 ){
      //Buffer类的使用
      //写完多少字节
      //需要移动多少字节
      outputBuffer_.retrive(n);
      //在全部将字节写出的时候
      //调用写完全的回调
      if(outputBuffer_.readableBytes() == 0){
        channel_->disableWriting();
        if(writeCompleteCallback_){
          //所有的回调函数都需要扔给Loop
          //loop分发给channel
          loop_->queueInLoop(std::bind(&TcpConnection::writeCompleteCallback_,shared_from_this()));
        }
        //在Kdisconnecting并且没有字节可以写的时候调用
        if(state_ == KDisConnecting){
          shutdownInLoop();
        }
      }
    }
    else{
      LOG_SYSERR<<"TcpConnection :: handleWrite()";
    }
  }
  else{
    LOG_SYSERR<<"Connection fd = "<<channel_->fd()<<"is down ,no more writing";
  }
}

void TcpConnection::handleClose(){
  loop_->assertInLoopThread();
  LOG_TRACE<<"fd = "<<channel_->fd()<<" state = "<<stateTostring();
  assert(state_ == KConnected || KDisConnecting);
  setState(KDisconnected);
  channel_->disAbleAll();
  TcpConnectionPtr graud(shared_from_this());
  connectionCallback_(graud);
  //处理close的时候调用closecallback
  //一种是绑定为loop_->(queueInloop)Tcp::connection::connectionDestory
  closeCallback_(graud);
}

void TcpConnection::handleError(){
  int err = sockets::getSocketError(channel_->fd());
  LOG_ERROR << "TcpConnection::handleError [" << name_
            << "] - SO_ERROR = " << err << " " << strerror_tl(err);
}



