#pragma once
//���಻�ɿ���������
//���ڷ�ֹһЩ���ñ���������Ķ�����ʽ��������
class nocopyable{
public:
    nocopyable(const nocopyable &) = delete;
    void operator=(const nocopyable &) = delete;
protected:
    nocopyable() = default;
    ~nocopyable() = default;
};