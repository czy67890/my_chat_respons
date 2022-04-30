#pragma once
#include"../base/string_piece.h"
#include"../base/string_piece.h"
#include"types.h"
#include"endian.h"
#include<algorithm>
#include<vector>
#include<assert.h>
#include<string.h>

namespace czy{
namespace net{
//Buffer��������csdn
//����֮
//һ��readIndex,һ��writeIndex,������Buffer�ֳ���������
//Ȼ�������������ֽ��в���

class Buffer{
public:
    static const size_t kCheapPrepend = 8;//Ԥ����prepend
    static const size_t kInittialSize = 1024;
    explicit Buffer(size_t initsize = kInittialSize)
     :buffer_(kCheapPrepend+initsize),readIndex_(kCheapPrepend),
     writeIndex_(kCheapPrepend)
    {
        assert(readableBytes() == 0);
        assert(writeableBytes() == initsize);
        assert(prependableBytes() == kCheapPrepend);
    }
    void swap(Buffer & rhs){
        //ʹ���ػ���swap�����������������
        buffer_.swap(rhs.buffer_);
        std::swap(writeIndex_,rhs.writeIndex_);
        std::swap(readIndex_,rhs.readIndex_);
    }
    size_t readableBytes() const{
        return writeIndex_-readIndex_;
    }
    size_t writeableBytes() const{
        return buffer_.size() - writeIndex_;
    }
    
    size_t prependableBytes() const{
        return readIndex_;
    }


    //���ض���ָ��
    const char * peek() const{
        return begin() + readIndex_;
    }
    const char * beginWrite() const{
        return begin() + writeIndex_;
    }
    const char * findCRLF() const{
        //std::search����Ѱ��������
        const char * crlf = std::search(peek(),beginWrite(),kCRLF,kCRLF+2);
        return crlf == beginWrite() ? NULL:crlf;
    }

    const char *findCRLF(const char* start) const{
        assert(peek() <= start);
        assert(start <= beginWrite());
        const char *crlf = std::search(start,beginWrite(),kCRLF,kCRLF+2);
        return crlf == beginWrite() ? NULL :crlf;
    }

    const char * findEOL(){
        //memchr��strchr ��Ч��Ҫ��
        //mem����ֱ�Ӳ����ڴ�
        //����memchr����'\0'������ͣ��
        const void * eol = memchr(peek(),'\n',readableBytes());
        return static_cast<const char *>(eol);
    }

    //��ͷ���ſ�һЩ����
    //���ڷ���һЩintֵ
    void retrive(size_t len){
        assert(len <= readableBytes());
        if(len <readableBytes()){
            readIndex_ += len;
        }
        else{
            retriveAll();
        }
    }

    void retriveUntil(const char * end){
        assert(peek() <= end);
        assert(end<= beginWrite());
        retrive(end - peek());
    }

    void retriveInt64(){
        retrive(sizeof(int64_t));
    }

    void retriveInt32(){
        retrive(sizeof(int32_t));
    }

    void retriveInt16(){
        retrive(sizeof(int16_t));
    }

    void retriveInt8(){
        retrive(sizeof(int8_t));
    }
    void retriveAll(){
        readIndex_ = writeIndex_ = kCheapPrepend;
    }

    string retriveAllAsString(){
        return retriveAsString(readableBytes());
    }
    string retriveAsString(size_t len){
        assert(len <= readableBytes());
        string result(std::move(string(peek(),len)));
        retrive(len);
        return result;
    }


private:
    //��ȡbegin�ٻ���ֵ
    //���ȡ��ַ
    char *begin(){
        return &*buffer_.begin();
    }

    const char * begin() const{
        return  &*buffer_.begin();
    }
    //�������ɴ�ŵĽ���ʱ
    //����
    //���򽫿ռ����µ���
    //���ɶ�����������ײ�
    void makeSpace(size_t len){
        if(writeableBytes() + prependableBytes() < len +kCheapPrepend){
            //ʹ��reserve �������
            buffer_.reserve(writeIndex_+len);
        }
        else{
            assert(kCheapPrepend < readIndex_);
            size_t readable = readableBytes();
            std::copy(begin() + readIndex_,begin() + writeIndex_
                ,begin()+kCheapPrepend
            );
            readIndex_ = kCheapPrepend;
            writeIndex_ = readIndex_ + readable;
            assert(readable == readableBytes());
        }
    }
private:
    std::vector<char> buffer_;
    size_t readIndex_;
    size_t writeIndex_;
    static const char kCRLF[];
};
}
}