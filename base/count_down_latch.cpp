#include"count_down_latch.h"
#include<exception>
CountDownLatch::CountDownLatch(int count) :count_(count){}
void CountDownLatch::wait(){
    //����ס,������RAII��ʵ�ַ�ֹ���ǽ���
    
    MutexGround lock(mutex_);
    while(count_ > 0){
       cond_.wait(mutex_);
    } 
}
void CountDownLatch::count_down(){
    MutexGround lock(mutex_);
    --count_;
    if(count_ == 0){
        cond_.notify_all();
    }
}
int CountDownLatch::get_count() const{
    MutexGround lock(mutex_);
    return count_;
}
