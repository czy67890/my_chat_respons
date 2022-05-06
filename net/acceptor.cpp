#include"acceptor.h"
#include"../base/logging.h"
#include"event_loop.h"
#include"inetaddress.h"
#include"socketops.h"

#include<errno.h>
#include<fcntl.h>
#include<unistd.h>

using namespace czy;
using namespace czy::net;

Acceptor::Acceptor(EventLoop *loop,const InetAddress &listenAddr,bool reuseport)
 :loop_(loop),
    acceptSocket_(sockets::createNonblockingOrDie(listenAddr.family())),
    //�������tcpserver��accept����channel�ַ�
    acceptChannel_(loop, acceptSocket_.fd()),
    listening_(false),
    //O_CLOEXEC:
    //��ִ��execϵͳ���õ�ʱ�򣬹ر��ļ�������
    //��ֹ�����̵��ļ����������ݸ��ӽ��̣������ӽ��̷���û��Ȩ�޵��ļ�
    idleFd_(::open("/dev/null", O_RDONLY | O_CLOEXEC))
{
    assert(idleFd_ >= 0);
    acceptSocket_.set_reuseable_addr(true);
    acceptSocket_.set_reuseadble_port(reuseport);
    acceptSocket_.bind_addr(listenAddr);
    acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead,this));
}

Acceptor::~Acceptor(){
    acceptChannel_.disAbleAll();
    acceptChannel_.remove();
    ::close(idleFd_);
}

void Acceptor::listen(){
    loop_->assertInLoopThread();
    listening_ = true;
    acceptSocket_.listen();
    acceptChannel_.enableReading();
}

//Acceptor��ְ�𣬼����µĵ�������
//�ڵ���accept�������������
//ȷ���м�û�з��������
//����newConnectionCallback_
//���ﱻTcpServer����Ϊ���е������еķ���newConnection
//������TcpServer��map��
void Acceptor::handleRead(){
    loop_->assertInLoopThread();
    InetAddress peerAddr;
    int connfd = acceptSocket_.accept(&peerAddr);
    if(connfd >= 0){
        if(newConnectionCallback_){
            newConnectionCallback_(connfd,peerAddr);
        }
        else{
            sockets::close(connfd);
        }
    }
    else{
        LOG_SYSERR<<"in Acceptor:: handleRead";
        //EMFILE��ʾ���ļ��������ﵽ���
        if(errno == EMFILE){
            ::close(idleFd_);
            idleFd_ = ::accept(acceptSocket_.fd(),NULL,NULL);
            ::close(idleFd_);
            idleFd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
        }
    }
}