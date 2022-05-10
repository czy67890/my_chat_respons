#include"m_thread.h"
#include"current_thread.h"
#include"logging.h"
#include <type_traits>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <linux/unistd.h>
namespace czy{

namespace detail{
pid_t get_tid(){
    return static_cast<pid_t>(::syscall(SYS_gettid));
}

void after_fork(){
    czy::CurrentThread::t_cached_tid = 0;
    czy::CurrentThread::t_thread_name = "main";
    CurrentThread::tid();
}
class ThreadNameInit{
public:
    ThreadNameInit(){
        czy::CurrentThread::t_thread_name = "main";
        CurrentThread::tid();
        //pthread_atfork()������Ϊ���ڶ��߳���fork�½���ʱ����������ǰ��һ���Ե�һ���ֶ�
        //��Ϊ�����׶�:prepare,parent,child
        pthread_atfork(NULL,NULL,&after_fork);
    }
};
ThreadNameInit init;
struct ThreadData{
    typedef czy::MThread::ThreadFunc ThreadFunc;
    ThreadFunc m_func;
    std::string m_name;
    pid_t * m_tid;
    CountDownLatch *m_latch;
    ThreadData(ThreadFunc func,const std::string &name,
    pid_t *tid,CountDownLatch *latch):
    m_func(std::move(func)),m_name(name),m_tid(tid),
    m_latch(latch)
    {}
    void run_in_thread(){
        czy::CurrentThread::t_thread_name = m_name.empty()?"unkonwn":m_name.c_str();
        //prctl���������߳����֣����������ں����ĵ���
        ::prctl(PR_SET_NAME,czy::CurrentThread::t_thread_name);
        try{
            m_func();
            czy::CurrentThread::t_thread_name = "finished";
        }    
        catch(const std::exception &ex){
            czy::CurrentThread::t_thread_name = "crashed";
            fprintf(stderr,"exception caught in Thread %s\n",m_name);
            fprintf(stderr, "reason: %s\n", ex.what());
            abort();
        }
    }
};
void* start_thread(void *obj){
    ThreadData *data = static_cast<ThreadData*> (obj);
    data->run_in_thread();
    delete data;
    return NULL;
}
}//end of namespace detail
void CurrentThread::cache_tid(){
    if(t_cached_tid == 0){
        //���ǵ�һ�Σ��򻺴����̱߳��
        t_cached_tid = detail::get_tid();
        t_tid_length = snprintf(t_tid_string,sizeof(t_tid_string),"%5d ",t_cached_tid);
    }
}
//�����Ƿ�Ϊmain�߳�
bool CurrentThread::is_main_thread(){
    return tid() == getpid();
}
void CurrentThread::sleep_usec(int64_t usec){
    struct timespec ts = {0,0};
    ts.tv_sec = static_cast<time_t>(usec/TimeStamp::k_micro_seconds_persecond);
    ts.tv_nsec = static_cast<long>(usec% TimeStamp::k_micro_seconds_persecond * 1000);
    ::nanosleep(&ts,NULL);
}
AtomicInt32 m_thread_creted;
//ʹ��move��ֹ�����ķ���
MThread::MThread(ThreadFunc func,const std::string &name):m_started(false),
m_joined(false),m_func(std::move(func)),m_name(name),m_pthread_id(0),m_tid(0)
,m_latch(1){
    set_default_name();
}
//����ʹ��pthread_detached��ʹ�߳��Լ�����
MThread::~MThread(){
    if(m_started&&!m_joined){
        pthread_detach(m_pthread_id);
    }
}
void MThread::set_default_name(){
    int num = m_thread_creted.add_and_get(1);
    if(m_name.empty()){
        char buf [32];
        snprintf(buf,sizeof(buf),"Thread %d",num);
        m_name = buf;
    }
}
void MThread::start(){
    assert(!m_started);
    m_started = true;
    //����Ҫ��ָ�룬���⿽��
    detail::ThreadData* new_data = new detail::ThreadData(m_func,m_name,&m_tid,&m_latch);
    if(pthread_create(&m_pthread_id,NULL,&detail::start_thread,new_data)){
        m_started = false;
        delete new_data;

    }
    else{
        m_latch.wait();
        assert(m_tid>0);
    }
}
 int MThread::jion(){
    assert(m_started);
    assert(!m_joined);
    m_joined = true;
    return pthread_join(m_pthread_id,NULL);
}
}
