#include"count_down_latch.h"
#include<exception>
namespace czy{
CountDownLatch::CountDownLatch(int count) :count_(count),mutex_(),cond_(mutex_){
    
}
void CountDownLatch::wait(){
    //先锁住,用守卫RAII来实现防止忘记解锁
    MutexGroud lock(mutex_);
    while(count_ > 0){
       cond_.wait();
    } 
}
void CountDownLatch::count_down(){
    MutexGroud lock(mutex_);
    --count_;
    if(count_ == 0){
        cond_.notify_all();
    }
}
int CountDownLatch::get_count() const{
    MutexGroud lock(mutex_);
    return count_;
}
}