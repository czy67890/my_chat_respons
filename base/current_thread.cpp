#include"current_thread.h"
#include<cxxabi.h>
namespace czy{

namespace CurrentThread{
__thread int t_cached_tid = 0;
__thread char t_tid_string[32];
__thread int t_tid_length = 6;
__thread const char * t_thread_name = "unkonwn";
//静态断言，可以在编译期检测出错误
static_assert(std::is_same<int,pid_t>::value,"pid_t should be int");
std::string back_trace(bool demangle){
    std::string stack;
    //最大的追溯帧数
    const int max_frame = 200;
    void *frame[max_frame];
    //获取帧的指针，存入frame中
    int nptrs = ::backtrace(frame,max_frame);
    //获取string数组
    char ** strings = ::backtrace_symbols(frame,nptrs);
    if(strings){
        size_t len = 256;
        //在demangle的情况下，创建内存区
        char *demangled = demangle?static_cast<char *>(::malloc(len)):nullptr;
        for(int i = 1;i<nptrs;++i){
            if(demangle){
                char *left_par = nullptr;
                char *plus = nullptr;
                for(char *p = strings[i];*p;++p){
                    if(*p == '('){
                        left_par = p;
                    }
                    else if(*p == '+'){
                        plus = p;
                    }
                }
                if(left_par && plus){
                    //先将+号置零
                    *plus = '\0';
                    int status = 0;
                    //获得类型名
                    char* ret = abi::__cxa_demangle(left_par+1,demangled,&len,&status);
                    //将plus复位
                    *plus = '+';
                    if(status == 0){
                        demangled = ret;
                        stack.append(strings[i],left_par+1);
                        stack.append(demangled);
                        stack.append(plus);
                        stack.push_back('\n');
                        continue;
                    }
                }
            }
            stack.append(strings[i]);
            stack.push_back('\n');
        }
        free(demangled);
        free(strings);
    }
    return stack;
}
}
}