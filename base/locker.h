#pragma once
#include "platform.h"
#include "raii_wrape.h"
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
private:
    //»¥³âËø
    pthread_mutex_t mutex_;
};
//·â×°ºÃRAII·ÀÖ¹Íü¼Ç½âËø
class MutexGroud {
public:
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
    Mutex& mutex_;
};
class Cond{
public:
    Cond(){
        if(pthread_cond_init(&cond_,NULL) != 0){
            throw std::exception();
        }
    }
    void lock(MutexGroud &mutex_){
        pthread_cond_wait(&cond_,mutex_.get_mutex().get_lock());
    }
    void post(){
        pthread_cond_signal(&cond_);
    }
    void notify_all(){
        pthread_cond_broadcast(&cond_);
    }
private:
    pthread_cond_t cond_;
};