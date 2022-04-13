#pragma once
#include"types.h"
#include <execinfo.h>   
#include<string>
//currentthread仅仅只是一个命名空间，而并不是一个实体类，封装了对
//线程的一些可供直接访问的量
//方便调试
namespace czy{

namespace CurrentThread{
    extern __thread int t_cached_tid;
    extern __thread char t_tid_string[32];
    extern __thread int t_tid_length;
    extern __thread const char * t_thread_name;
    void cache_tid();
    inline int tid(){
        //__builtin_expect()函数帮助我们做分支预测,如果函数内条件成立
        //那么进行
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