#pragma once
#include<stdlib.h>
#include<sys/types.h>
#include<string>
#include<sstream>
#include<stdint.h>


//二进制的包类型
//用于内部服务器间的通信
namespace net{
    enum{
        TEXT_PACKLEN_LEN = 4,
        TEXT_PACKAGE_MAXLEN = 0xffff,
        BINARY_PACKAGE_LEN = 2,
        BINARY_PACKAGE_MAXLEN = 0xffff,
        
        TEXT_PACKLEN_LEN_2 = 6,
        TEXT_PACKAGE_MAXLEN_2 = 0xffffff,

        BINARY_PACKLEN_LEN_2 = 4,               //4字节头长度
        BINARY_PACKAGE_MAXLEN_2 = 0x10000000,   //包最大长度是256M,足够了

        CHECKSUM_LEN = 2,
    };

    unsigned int checkSum(const unsigned short* buf,int size);

}