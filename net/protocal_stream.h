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
    //计算校验和
    unsigned int checkSum(const unsigned short* buf,int size);
    //
    void write7BitEncoded(uint32_t value,std::string &buf);

    void write7BitEncoded(uint64_t value,std::string &buf);

    void read7BitEncoded(const char * buf,uint32_t len,uint32_t &value);

    void read7BitEncoded(const char * buf,uint32_t len,uint64_t &value);


    //final 用于类代表当前类不能作为基类被重写
    class BinaryStreamReader final{
    public:
        BinaryStreamReader(const char * ptr,int len);
        ~BinaryStreamReader() = default;

        virtual const char * GetData() const;
        virtual size_t GetSize() const;
        bool isEmpty() const;
        bool readString(std::string * str,size_t maxlen,size_t& outlen);
        bool readCString(char * str,size_t strlen,size_t &len);
        bool readCCstring(const char ** str,size_t maxlen,size_t&outlen);
        bool readInt32(int32_t& i);
        bool readInt64(int64_t& i);
        
    };
}