#pragma once
#include<functional>
#include"count_down_latch.h"
#include"nocopyable.h"
#include<assert.h>
#include"atomic.h"
#include"time_stamp.h"
//�Լ����߳���
namespace czy{
class MThread : nocopyable{
public:
    //�߳���Ҫ���еĺ���
    typedef std::function<void()> ThreadFunc;
    //�����캯������Ϊexplict�ķ�ֹĬ��ת��
    explicit MThread(ThreadFunc,const std::string &name = std::string());
    ~MThread();
    void start();
    bool started(){return m_started;}
    int jion();
    pid_t tid() const {return m_tid;};
    static int num_created(){return m_num_created.get();}
    //����Ϊconst�ķ�ֹ�����޸�
    bool started() const {return m_started;}
private:
    void set_default_name();
    bool m_started;
    bool m_joined;
    pid_t m_tid;
    pthread_t m_pthread_id;
    ThreadFunc m_func;
    std::string m_name;
    CountDownLatch m_latch;
    static czy::AtomicInt32 m_num_created;
};
}