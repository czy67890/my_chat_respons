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
//Buffer的设计详见csdn
//简言之
//一个readIndex,一个writeIndex,将整个Buffer分成三个部分
//然后对这儿三个部分进行操作

class Buffer{
public:
    static const size_t kCheapPrepend = 8;//预留的prepend
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
        //使用特化的swap函数，可以提高性能
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


    //返回读的指针
    const char * peek() const{
        return begin() + readIndex_;
    }
    char * beginWrite(){
        return begin() + readIndex_;
    }
    const char * beginWrite() const{
        return begin() + writeIndex_;
    }

    void hasWritten(size_t len){
        assert(len <= writeableBytes());
        writeIndex_ += len;
    }

    void unwrite(size_t len){
        assert(len <= readableBytes());
        writeIndex_ -= len;
    }
    const char * findCRLF() const{
        //std::search用于寻找子序列
        const char * crlf = std::search(peek(),beginWrite(),kCRLF,kCRLF+2);
        return crlf == beginWrite() ? NULL:crlf;
    }

    const char *findCRLF(const char* start) const{
        assert(peek() <= start);
        assert(start <= beginWrite());
        const char *crlf = std::search(start,beginWrite(),kCRLF,kCRLF+2);
        return crlf == beginWrite() ? NULL :crlf;
    }

    const char * findEOL() const{
        //memchr比strchr 的效率要高
        //mem函数直接操作内存
        //而且memchr遇到'\0'并不会停下
        const void * eol = memchr(peek(),'\n',readableBytes());
        return static_cast<const char *>(eol);
    }

    const char *findEOL(const char * start){
        assert(start < beginWrite());
        assert(start >= peek());
        const void* eol = memchr(start,'\n',beginWrite()-start);
        return static_cast<const char *>(eol);
    }


    //将头部排空一些距离
    //这样写是获取到头部的一些int值
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
    
    StringPiece toStringPiece() const{
        return StringPiece(peek(),static_cast<int> (readableBytes()));
    }

    void append(const StringPiece &str){
        append(str.data(),str.size());
    }
    
    void append(const char * data,size_t len){
        ensureWriteableBytes(len);
        std::copy(data,data+len,beginWrite());
        hasWritten(len);
    }

    void append(const void * data,size_t len){
        append(static_cast<const char *>(data) ,len);
    }

    void ensureWriteableBytes(size_t len){
        if(writeableBytes() < len){
            makeSpace(len); 
        }
        assert(writeableBytes() >= len);
    }
void appendInt64(int64_t x)
  {
    int64_t be64 = sockets::hostToNetwork64(x);
    append(&be64, sizeof be64);
  }

  ///
  /// Append int32_t using network endian
  ///
  void appendInt32(int32_t x)
  {
    int32_t be32 = sockets::hostToNetwork32(x);
    append(&be32, sizeof be32);
  }

  void appendInt16(int16_t x)
  {
    int16_t be16 = sockets::hostToNetwork16(x);
    append(&be16, sizeof be16);
  }

  void appendInt8(int8_t x)
  {
    append(&x, sizeof x);
  }

  int64_t readInt64()
  {
    int64_t result = peekInt64();
    retriveInt64();
    return result;
  }

  ///
  /// Read int32_t from network endian
  ///
  /// Require: buf->readableBytes() >= sizeof(int32_t)
  int32_t readInt32()
  {
    int32_t result = peekInt32();
    retriveInt32();
    return result;
  }

  int16_t readInt16()
  {
    int16_t result = peekInt16();
     retriveInt16();
    return result;
  }

  int8_t readInt8()
  {
    int8_t result = peekInt8();
    retriveInt8();
    return result;
  }
 int64_t peekInt64() const
  {
    assert(readableBytes() >= sizeof(int64_t));
    int64_t be64 = 0;
    ::memcpy(&be64, peek(), sizeof be64);
    return sockets::networkToHost64(be64);
  }

  ///
  /// Peek int32_t from network endian
  ///
  /// Require: buf->readableBytes() >= sizeof(int32_t)
  int32_t peekInt32() const
  {
    assert(readableBytes() >= sizeof(int32_t));
    int32_t be32 = 0;
    ::memcpy(&be32, peek(), sizeof be32);
    return sockets::networkToHost32(be32);
  }

  int16_t peekInt16() const
  {
    assert(readableBytes() >= sizeof(int16_t));
    int16_t be16 = 0;
    ::memcpy(&be16, peek(), sizeof be16);
    return sockets::networkToHost16(be16);
  }

  int8_t peekInt8() const
  {
    assert(readableBytes() >= sizeof(int8_t));
    int8_t x = *peek();
    return x;
  }
  //prenpend向后追加一些数据
    void prepend(const void * data,size_t len){
        assert(len <= prependableBytes());
    }
  ///
  /// Prepend int64_t using network endian
  ///
   void prependInt64(int64_t x)
  {
    int64_t be64 = sockets::hostToNetwork64(x);
    prepend(&be64, sizeof be64);
  }

  ///
  /// Prepend int32_t using network endian
  ///
  void prependInt32(int32_t x)
  {
    int32_t be32 = sockets::hostToNetwork32(x);
    prepend(&be32, sizeof be32);
  }

  void prependInt16(int16_t x)
  {
    int16_t be16 = sockets::hostToNetwork16(x);
    prepend(&be16, sizeof be16);
  }

  void prependInt8(int8_t x)
  {
    prepend(&x, sizeof x);
  }

  void shirnk(size_t reserve){
      Buffer other;
      other.ensureWriteableBytes(readableBytes() + reserve);
      other.append(toStringPiece());
      swap(other);
  }

  size_t internalCapacity(){
      return buffer_.capacity();
  }

  ssize_t readFd(int fd,int *savedErrorno);
private:
    //先取begin再换成值
    //最后取地址
    char *begin(){
        return &*buffer_.begin();
    }

    const char * begin() const{
        return  &*buffer_.begin();
    }
    //当超过可存放的界限时
    //扩容
    //否则将空间重新调整
    //将可读区域调整
    void makeSpace(size_t len){
        if(writeableBytes() + prependableBytes() < len +kCheapPrepend){
            //使用reserve 提高性能
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