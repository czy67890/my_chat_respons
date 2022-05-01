#pragma once
#include "platform.h"
#include "raii_wrape.h"
#include"current_thread.h"
#include"nocopyable.h"
namespace czy{
class Mutex{
public:
    Mutex(){
        if(pthread_mutex_init(&mutex_,NULL) != 0){
            throw std::exception();
        }
    }
    bool lock(){
        return pthread_mutex_lock(&mutex_) == 0;
    }
    bool unlock(){
        return pthread_mutex_unlock(&mutex_) == 0;
    }
    ~Mutex(){
        if(pthread_mutex_destroy(&mutex_) != 0){
            throw std::exception();
        }
    }
    pthread_mutex_t *get_lock(){
        return &mutex_;
    }
    //?Cond??????????
    friend class Cond;
private:
    //»¥³âËø
    pthread_mutex_t mutex_;
    pid_t m_holder;
    class UnassignGuard : nocopyable{
    public:
        explicit UnassignGuard(Mutex & woner)
         :m_owner(woner)
        {
            m_owner.unassign_holder();
        }
        ~UnassignGuard(){
            m_owner.assign_holder();
        }
    private:
      Mutex& m_owner;  
    };
    void unassign_holder(){
        m_holder = 0;
    }
    void assign_holder(){
        m_holder = CurrentThread::tid();
    }
};
//·â×°ºÃRAII·ÀÖ¹Íü¼Ç½âËø
class MutexGroud {
public:
    //????????????????
    MutexGroud(Mutex &mutex):mutex_(mutex){
        mutex.lock();
    }
    ~MutexGroud(){
        mutex_.unlock();
    }
    Mutex& get_mutex(){
        return mutex_;
    }
private:
    //????????Mutex???
    //???????????
    //????????
    Mutex& mutex_;
};
class Cond{
public:
    //?????????????????
    Cond(Mutex & mutex):m_mutex(mutex){
        if(pthread_cond_init(&cond_,NULL) != 0){
            throw std::exception();
        }
    }
    void wait(){
        pthread_cond_wait(&cond_ , m_mutex.get_lock());
    }
    void post(){
        pthread_cond_signal(&cond_);
    }
    void notify_all(){
        pthread_cond_broadcast(&cond_);
    }
    bool wait_for_seconds(double seconds){
        //timespec????????????
        //?linux???????
        struct timespec abstime;
        //CLOCK_REALTIME??????????????
        //????CLOCK_MONOTONIC
        clock_gettime(CLOCK_MONOTONIC,&abstime);
        const int64_t k_nanosecond_per_second = 1000000000;
        int64_t nano_second = static_cast<int64_t>(seconds * k_nanosecond_per_second);  
        abstime.tv_sec += static_cast<time_t>(abstime.tv_nsec + nano_second)/k_nanosecond_per_second;
        abstime.tv_nsec = static_cast<long>((abstime.tv_nsec + nano_second))%k_nanosecond_per_second;
        Mutex::UnassignGuard ug(m_mutex);
        //errno
        //pthread_cond_timewait()????????
        return ETIMEDOUT == pthread_cond_timedwait(&cond_,m_mutex.get_lock(),&abstime);
    }
private:
    Mutex &m_mutex;
    pthread_cond_t cond_;
};
}