#pragma once
//基类不可拷贝构造类
//用于防止一些不该被拷贝构造的对象被隐式拷贝构造
class nocopyable{
public:
    //并且注意需要申明成public的
    //这是由于c++的校验是先验证可达性，再验证是否删除的
    //若申明成private编译器只会给出访问不可达
    //将拷贝构造和赋值构造函数设置成delete的
    //这种方法会比定义成private然后只申明而不定义好很多
    nocopyable(const nocopyable &) = delete;
    void operator=(const nocopyable &) = delete;
protected:
    nocopyable() = default;
    ~nocopyable() = default;
};