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
private:
    //»¥³âËø
    pthread_mutex_t mutex_;
};
//·â×°ºÃRAII·ÀÖ¹Íü¼Ç½âËø
class MutexGroud :public RAIIWrapper{
public:
    MutexGround(pthread_mutex_t& lcok) mutex_(lock){
        pthread_mutex_lock(lock);
    }
    ~MutexGroud(){
        pthread_mutex_unlock(&mutex);
    }
private:
    pthread_mutex_t mutex_;
};
class Cond{
public:
    
private:
    pthread_cond_t cond;
};