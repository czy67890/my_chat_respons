#pragma once
#include<map>
#include<string>
#include<string.h>
class ConfigureFileReader
{
private:
    bool                               m_load_ok;
    std::map<std::string,std::string>  m_configure_map;
    std::string                        m_file_name;
    //���ļ��е�����д��map��
    void Load_file(const char *file_name);
    //��map�е�����д���ļ���
    int write_file(const char *file_name = NULL);
    //����ȡ�����ļ���ת����map���Խ��ܵĸ�ʽ
    void parse_line(char *line);
    //ȥ�����еķǱ�Ҫ��ʽ
    char* trim_space(char *text);
public:
    ConfigureFileReader(const char* file_name);
    //�����ļ���
    char *get_configure_name(const char *name);
    //������д������
    int set_configure_value(char *name,char *value);
};

