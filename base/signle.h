#ifndef _SIGEL_H_
#define _SIGEL_H_
template<typename T>
//����ģʽʵ��
class Singleton{
public:
    //����ģʽ,���ص������ã�����ʵ���̲߳���ȫ����Ϊ����ʽ��
    //�̰߳�ȫ
    static T &instance(){
        return value_;
    }
private:
    Singleton();
    ~Singleton() = default;
    //��������������Ϊɾ����
    Singleton(const Singleton &) = delete;
    Singleton& operator=(const Singleton &) = delete;
    static void destroy(){
        delete value_;
        value_ = NULL;
    }
    static T *value_;
};
template<typename T>
T *Singleton<T>::value_ = new T();
#endif