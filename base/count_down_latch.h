#ifndef COUNT_DOWN_H_
#define COUNT_DOWN_H_
#include "platform.h"
#include"locker.h"
class CountDownLatch{
public:
    explicit CountDownLatch(int count);
    void wait();
    void count_down();
    int get_count() const;
private:
    //设置成mutable是为了和可能的const连用
    mutable pthread_mutex_t mutex_;
    pthread_cond_t cond_;
    int count_;
};
#endif