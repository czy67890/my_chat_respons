#ifndef COUNT_DOWN_H_
#define COUNT_DOWN_H_
#include "platform.h"
class CountDownLatch{
public:
    explicit CountDownLatch(int count);
    void wait();
    void count_down();
    int get_count() const;
private:
    //���ó�mutable��Ϊ�˺Ϳ��ܵ�const����
    mutable pthread_mutex_t mutex_;
    pthread_cond_t cond_;
    int count_;
};
#endif