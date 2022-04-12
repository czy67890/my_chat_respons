#pragma once
//基类不可拷贝构造类
//用于防止一些不该被拷贝构造的对象被隐式拷贝构造
class nocopyable{
public:
    nocopyable(const nocopyable &) = delete;
    void operator=(const nocopyable &) = delete;
protected:
    nocopyable() = default;
    ~nocopyable() = default;
};