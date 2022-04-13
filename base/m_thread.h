#pragma once
#include"platform.h"
#include<functional>
#include"count_down_latch.h"
#include"nocopyable.h"
#include<assert.h>
#include"atomic.h"
#include"time_stamp.h"
//自己的线程类
namespace czy{
class MThread : nocopyable{
public:
    //线程需要运行的函数
    typedef std::function<void()> ThreadFunc;
    //将构造函数申明为explict的防止默认转型
    explicit MThread(ThreadFunc,const std::string &name = std::string());
    ~MThread();
    void start();
    bool started(){return m_started;}
    int jion();
    pid_t tid() const {return m_tid;};
    static int num_created(){return m_num_created.get();}
    //声明为const的防止意外修改
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