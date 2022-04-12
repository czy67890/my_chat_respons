#pragma once
#include"platform.h"
#include<functional>
#include"count_down_latch.h"
#include"atomic.h"
//自己的线程类
class MThread{
public:
    //线程需要运行的函数
    typedef std::function<void()> ThreadFunc;
    //将构造函数申明为explict的防止默认转型
    explicit MThread(ThreadFunc,const std::string &name = std::string());
    ~MThread();
    void start();
    void jion();
    pid_t get_pid();
    
    //声明为const的防止意外修改
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