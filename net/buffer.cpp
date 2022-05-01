#include"buffer.h"
#include"socketops.h"
#include<errno.h>
#include<sys/uio.h>

using namespace czy;
using namespace czy::net;

const char Buffer::kCRLF[] = "\r\n";

const size_t Buffer::kCheapPrepend;
const size_t Buffer::kInittialSize;

ssize_t Buffer::readFd(int fd,int *savederrorno){
    char extrabuf[65536];
    //iovec用于分散读集中写
    struct iovec vec[2];
    const size_t writeable = writeableBytes();
    vec[0].iov_base = begin() + writeIndex_;
    vec[0].iov_len = writeable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = 65536;

    const int iovcnt = (writeable < sizeof(extrabuf) ?2:1);

    const ssize_t n = sockets::readv(fd,vec,iovcnt);

    if(n<0){
        *savederrorno = errno;
    }
    else if(implicit_cast<size_t>(n) <= writeable){
        writeIndex_ += n;
    }
    else{
        writeIndex_ = buffer_.size();
        append(extrabuf,n-writeable);
    }
    return n;
}
