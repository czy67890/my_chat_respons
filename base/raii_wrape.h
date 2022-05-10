#ifndef _RAII_WARPE_H_
#define _RAII_WARPE_H_
//RAII��װ��һ������
//ģ����
template<typename T>
class RAIIWrapper{
public:
    RAIIWrapper(T * ptr):ptr_(ptr){
    }
    //��������������Ϊvirtual ǿ�Ƽ̳е�����ʵ��
    virtual  ~RAIIWrapper(){
        if(ptr_){
            //����֮������NULL
            delete ptr_;
            ptr_ = NULL;
        }
    }
private:
    T* ptr_;
};
#endif