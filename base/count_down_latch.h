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
    //���ó�mutable��Ϊ�˺Ϳ��ܵ�const����
    mutable Mutex mutex_;
    Cond cond_;
    int count_;
};
#endif