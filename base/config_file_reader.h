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
    //将文件中的内容写入map中
    void Load_file(const char *file_name);
    //将map中的内容写入文件中
    int write_file(const char *file_name = NULL);
    //将读取到的文件行转换成map可以接受的格式
    void parse_line(char *line);
    //去除行中的非必要格式
    char* trim_space(char *text);
public:
    ConfigureFileReader(const char* file_name);
    //返回文件名
    char *get_configure_name(const char *name);
    //往表中写入数据
    int set_configure_value(char *name,char *value);
};

