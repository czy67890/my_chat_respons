#pragma once
//���಻�ɿ���������
//���ڷ�ֹһЩ���ñ���������Ķ�����ʽ��������
class nocopyable{
public:
    //����ע����Ҫ������public��
    //��������c++��У��������֤�ɴ��ԣ�����֤�Ƿ�ɾ����
    //��������private������ֻ��������ʲ��ɴ�
    //����������͸�ֵ���캯�����ó�delete��
    //���ַ�����ȶ����privateȻ��ֻ������������úܶ�
    nocopyable(const nocopyable &) = delete;
    void operator=(const nocopyable &) = delete;
protected:
    nocopyable() = default;
    ~nocopyable() = default;
};