#pragma once
#include <stdint.h>
#include <string.h>  // memset
#include <string>
//implict_cast<>��һ�ְ�ȫ������ת������ʹ��static_cast<>
//��ô������ת��ʱ��Ȼ�ɹ�
//��ʱ�������ʱδ�����
//��������ʱcrash
//��dynam_cast��������RTII��������Ӧ�ñ����һ�����
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