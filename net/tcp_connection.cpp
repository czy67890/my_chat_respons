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
{   //ʹ��std::bind()ȷ��һ�����ɱ����������øûص�����
    //tcpconnection�����þ��ǽ�������
    //�Լ���Ϊchannel�ķ���ʵ��ȥִ������
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

//ֻ����ĳ�Ա�����ſ������������const�����������ĳ�Ա���������޸�
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
      //����ֱ����move����
      loop_->runInLoop(std::bind(fp,this,std::move(message.as_string())));
    }
  }
}

void TcpConnection::send(Buffer *buf){
  //�����ǰevent_loop���Ǳ��߳�
  //��ֱ�ӷ���
  //�����Ȼ�ȡ�����߳�
  //�ٷ���
  if(state_ == KConnected){
    if(loop_->isInLoopThread()){
      sendInLoop(buf->peek(),buf->readableBytes());
      buf->retriveAll();
    }
    else{
      //���Ǳ��߳�
      //��ô�Ȼ��ѱ��̣߳��ٵ���sendInLoop
      //ע����Щ������bind��������������
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
        //EWOULDBLOCK ʵ���ϵ���EAGAIN
        //
        if(errno != EWOULDBLOCK){
          LOG_SYSERR<<"TcpConnection:sendInLoop";
          //EPIPEͬSIG_PIPEһ��
          //�������Ĵλ���ʱ�����ͻ����Ѿ��ر�������
          //��serve���ڷ���
          //��ʱ�����EPIPE
          //ECONNRESET�����������ĳһ����������Ҫ�������ӵ����
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
    //һ��ԭ����TcpConnection������Ҫ�����������߳�ȥ����
    //���ʱ�����ǲ�Ӧ��ʹ��thisָ��ȥbind����ʹ��shared_from_thisȥbind
    //�������ܱ�֤�������ڲ������ں���ִ����֮ǰ����
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
  //channel ��tie�ڴ�������ʱ����
  //�����������shared_from_this
  //����ָ�����������������ڵİ���
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
    //��n == 0����Է��ѷ�����
    //���ʱ��ر�����
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
  //����ֱ�ӵ���sockets��write����
  if(channel_->isWriting()){
    ssize_t n = sockets::write(channel_->fd(),outputBuffer_.peek(),outputBuffer_.readableBytes());
    if(n > 0 ){
      //Buffer���ʹ��
      //д������ֽ�
      //��Ҫ�ƶ������ֽ�
      outputBuffer_.retrive(n);
      //��ȫ�����ֽ�д����ʱ��
      //����д��ȫ�Ļص�
      if(outputBuffer_.readableBytes() == 0){
        channel_->disableWriting();
        if(writeCompleteCallback_){
          //���еĻص���������Ҫ�Ӹ�Loop
          //loop�ַ���channel
          loop_->queueInLoop(std::bind(&TcpConnection::writeCompleteCallback_,shared_from_this()));
        }
        //��Kdisconnecting����û���ֽڿ���д��ʱ�����
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
  closeCallback_(graud);
}

void TcpConnection::handleError(){
  int err = sockets::getSocketError(channel_->fd());
  LOG_ERROR << "TcpConnection::handleError [" << name_
            << "] - SO_ERROR = " << err << " " << strerror_tl(err);
}



