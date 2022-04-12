#pragma once
#include"platform.h"
#include<functional>
#include"count_down_latch.h"
#include"atomic.h"
//�Լ����߳���
class MThread{
public:
    //�߳���Ҫ���еĺ���
    typedef std::function<void()> ThreadFunc;
    //�����캯������Ϊexplict�ķ�ֹĬ��ת��
    explicit MThread(ThreadFunc,const std::string &name = std::string());
    ~MThread();
    void start();
    void jion();
    pid_t get_pid();
    
    //����Ϊconst�ķ�ֹ�����޸�
    bool started() const {return m_started;}
private:
    bool m_started;
    pid_t m_tid;
    pthread_t m_pthread_id;
    ThreadFunc m_func;
    std::string m_name;
    CountDownLatch m_latch;
    static AtomicInt32 num_created_;
};