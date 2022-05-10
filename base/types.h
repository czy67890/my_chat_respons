#pragma once
#include <stdint.h>
#include <string.h>  // memset
#include <string>
//implict_cast<>是一种安全的类型转换，若使用static_cast<>
//那么会在下转型时任然成功
//这时候程序功能时未定义的
//会在运行时crash
//而dynam_cast则会在造成RTII这是我们应该避免的一种情况
namespace czy{
using std::string;
inline void mem_zero(void * p,size_t n){
    memset(p,0,n);
}
template<typename To,typename From>
inline To implicit_cast(From const &f){
    return f;
}
template<typename To,typename From>
inline To down_cast(From *f){
    if(false){
        implicit_cast<From*,To>(0);
    }
#if !defined(NDEBUG) && !defined(GOOGLE_PROTOBUF_NO_RTTI)
  assert(f == NULL || dynamic_cast<To>(f) != NULL);  // RTTI: debug mode only!
#endif
  return static_cast<To>(f);
}
}