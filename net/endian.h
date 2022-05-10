#pragma once 
extern "C"{
  #include "/usr/include/endian.h"
}

#include<stdint.h>
class abc{
public:
  abc();
};
//因为网络字节序列时大端模式
//而主机字节序列时小端模式
//这个头文件用于转换
namespace czy{
namespace net{
namespace sockets{
//内敛汇编代码使得类型模糊
//暂时屏蔽警告信息
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wold-style-cast"

inline uint64_t hostToNetwork64 (uint64_t host64){
    return htobe64(host64);
}
inline uint32_t hostToNetwork32(uint32_t host32)
{
  return htobe32(host32);
}

inline uint16_t hostToNetwork16(uint16_t host16)
{
  return htobe16(host16);
}

inline uint64_t networkToHost64(uint64_t net64)
{
  return be64toh(net64);
}

inline uint32_t networkToHost32(uint32_t net32)
{
  return be32toh(net32);
}

inline uint16_t networkToHost16(uint16_t net16)
{
  return be16toh(net16);
}
#pragma GCC diagnostic pop
}
}
}
