#include"config_file_reader.h"
ConfigureFileReader::ConfigureFileReader(const char*file_name){
    if(file_name){
        m_file_name.clear();
        m_file_name.append(file_name);
        Load_file(file_name);
    }
}
void ConfigureFileReader::Load_file(const char *file_name){
    FILE *file_to_read = fopen(file_name,"r");
    if(!file_to_read){
        return;
    }
    char buf[256];
    for(;;){
        //fgets方法，将file中的内容读入buf
        char *p = fgets(buf,256,file_to_read);
        if(!p){
            break;
        }
        size_t str_len = strlen(buf);
        if(buf[255] == '\n'){
            buf[255] = 0;
        }
        //去除开头的#
        char *ch = strchr(buf,'#');
        if(ch){
            *ch = 0;
        }
        if(strlen(buf) == 0){
            continue;
        }
        parse_line(buf);
    }
    //关闭文件描述符
    fclose(file_to_read);
    m_load_ok = true;
}
void ConfigureFileReader::parse_line(char *line){
    char * p =strchr(line,'=');
    *p = 0;
    char *key = trim_space(line);
    char *value = trim_space(p+1);
    if(key &&value){
        m_configure_map[key] = value;
    }
}
char * ConfigureFileReader::trim_space(char *text){
    char *start_pos = text;
    while(((*start_pos == ' ')||
    (*start_pos) == '\t'||(*start_pos) == '\r')){
        start_pos ++;
    }
    if(strlen(start_pos) == 0){
        return NULL;
    }
    char *end_pos = text + strlen(text) -1;
    while(((*end_pos == ' ')||
    (*end_pos) == '\t'||(*end_pos) == '\r')){
        end_pos = 0;
        end_pos --;
    }
    int len = (int)(end_pos - start_pos) + 1;
    if(len <= 0){
        return NULL;
    }
    return start_pos;
}
char *ConfigureFileReader::get_configure_name(const char *name){
    if(!m_load_ok){
        return NULL;
    }
    char * value = NULL;
    auto p = m_configure_map.find(name);
    if(p != m_configure_map.end()){
        value = (char *)p->second.c_str();
    }
    return value;
}
int ConfigureFileReader::write_file(const char *file_name){
    FILE *file_to_write = NULL;
    if(file_name == NULL){
        file_to_write = fopen(m_file_name.c_str(),"w");
    }
    else{
        file_to_write = fopen(file_name,"w");
    }
    if(file_to_write == NULL){
        return -1;
    }
    char sz_pair[128];
    for(auto p =m_configure_map.begin();p != m_configure_map.end();++p){
        memset(sz_pair,0,128);
        snprintf(sz_pair,128,"%s=%s",p->first.c_str(),p->second.c_str());
        //fwrite函数传入一个元素，返回写入成功的元素个数
        size_t ret = fwrite(sz_pair,strlen(sz_pair),1,file_to_write);
        if(ret != 1){
            fclose(file_to_write);
            return -1;
        }
    }
    fclose(file_to_write);
    return -1;
}