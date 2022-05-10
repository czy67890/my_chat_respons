#ifndef _RAII_WARPE_H_
#define _RAII_WARPE_H_
//RAII封装的一个基类
//模板编程
template<typename T>
class RAIIWrapper{
public:
    RAIIWrapper(T * ptr):ptr_(ptr){
    }
    //将析构函数设置为virtual 强制继承的类来实现
    virtual  ~RAIIWrapper(){
        if(ptr_){
            //析构之后设置NULL
            delete ptr_;
            ptr_ = NULL;
        }
    }
private:
    T* ptr_;
};
#endif