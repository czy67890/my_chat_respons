#pragma once
#include<functional>

#include"channel.h"
#include"sockets.h"

namespace czy{
namespace net{

class EventLoop;
class InetAddress;

//Acceptor����Ҫ���ܡ���������
//������Ҫʹ�����ļ����������ص�ʱ���ܹ���������������ʹ�ó�����Զͣ��
//����:���ȴ�һ�����ļ���ռ��һ��������
//���������������ռλ��
//���˵���ִ��ļ�����������Ĵ���ʱ
//�ȹر�ռλ��
//Ȼ�����ӵ����ռλ��
//��client��Ϊ�Լ����ӳɹ�
//Ȼ������ر�
//�����ø�ռλ���ص�ռ����ļ���������
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