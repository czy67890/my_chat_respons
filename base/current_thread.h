#pragma once
#include"types.h"
#include <execinfo.h>   
#include<string>
//currentthread����ֻ��һ�������ռ䣬��������һ��ʵ���࣬��װ�˶�
//�̵߳�һЩ�ɹ�ֱ�ӷ��ʵ���
//�������
namespace czy{

namespace CurrentThread{
    extern __thread int t_cached_tid;
    extern __thread char t_tid_string[32];
    extern __thread int t_tid_length;
    extern __thread const char * t_thread_name;
    void cache_tid();
    inline int tid(){
        //__builtin_expect()����������������֧Ԥ��,�����������������
        //��ô����
        if(__builtin_expect(t_cached_tid == 0,0)){
            cache_tid();
        }
        return t_cached_tid;
    }
    inline const char * tid_string(){
        return t_tid_string;
    }
    inline int tid_length(){
        return t_tid_length;
    }
    bool is_main_thread();
    void sleep_usec(int64_t usec);
    std::string stack_trace(bool demalge);
    
}

}