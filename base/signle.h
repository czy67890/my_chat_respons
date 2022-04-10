#ifndef _SIGEL_H_
#define _SIGEL_H_
template<typename T>
//单列模式实现
class Singleton{
public:
    //懒汉模式,返回的是引用，这种实现线程不安全，改为饿汉式，
    //线程安全
    static T &instance(){
        return value_;
    }
private:
    Singleton();
    ~Singleton() = default;
    //将拷贝构造设置为删除的
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