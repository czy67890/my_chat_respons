#include<string>
#include<iosfwd>
#include"types.h"
namespace czy{
class  StringArg{
public:
    StringArg(const char * str)
     :m_str(str)
    {}
    StringArg(const string &str):
    m_str(str.c_str())
    {}
    const char * c_str() const{return m_str;}
private:
    const char * m_str;
};
class StringPiece{
private:
    const char *m_ptr;
    int m_length;
public:
    StringPiece()
     :m_ptr(NULL),m_length(0){ }
    StringPiece(const char *str)
     :m_ptr(str),m_length(static_cast<int> (strlen(m_ptr))){ };
    StringPiece(const unsigned char * str)
      //reinterpret_cast 使用指针指向的地址，只转换其解释的方法
     :m_ptr(reinterpret_cast<const char *>(str)),
      m_length(static_cast<int> (strlen(m_ptr))){ }
    StringPiece(const string & str) 
     :m_ptr(str.c_str()),
     m_length(static_cast<int> (str.size()))
    { }
    StringPiece(const char *offset,int len)
     :m_ptr(offset),m_length(len){}
    const char *data() const{ return m_ptr;}
    int size() const { return m_length;}
    bool empty() const {return m_length == 0;}
    const char *begin() const {return m_ptr;}
    const char *end() const {return m_ptr+m_length;}
    void clear() {m_ptr = NULL; m_length = 0;}
    void set(const char * buffer,int length ){ m_ptr = buffer,m_length = length;}
    void set(const char * str){
        m_ptr = str;
        m_length = static_cast<int> (strlen(str));
    }
    bool operator[](int i) const {return m_ptr[i];}
    void remove_prefix(int n){
        m_ptr += n;
        m_length -= n;
    }
    void remove_suffix(int n){
        m_length -= n;
    }
    bool operator==(const StringPiece &x) const{
        return ((m_length == x.m_length)&&(memcmp(m_ptr,x.m_ptr,m_length) == 0));
    }
    bool operator!=(const StringPiece &x) const{
        return !(*this == x); 
    }
#define STRINGPIECE_BINARY_PREDICATE(cmp,auxcmp) \
    bool operator cmp(const StringPiece &x) const{ \
        int r = memcmp(m_ptr,x.m_ptr,m_length < x.m_length ? m_length:x.m_length) ;\
        return ((r auxcmp 0) ||((r == 0) && (m_length cmp x.m_length))); \
    } 
STRINGPIECE_BINARY_PREDICATE(<,  <);
STRINGPIECE_BINARY_PREDICATE(<=, <);
STRINGPIECE_BINARY_PREDICATE(>=, >);
STRINGPIECE_BINARY_PREDICATE(>,  >);
#undef STRINGPIECE_BINARY_PREDICATE
int compare(const StringPiece &x) const{
    int r = memcmp(m_ptr,x.m_ptr,m_length<x.m_length ?m_length : x.m_length);
        if(r == 0){
            if(m_length < x.m_length) r == -1;
            else if(m_length > x.m_length) r = +1;
        }
        return r;
    }
string as_string() const{
    return string(data(),size());
}
    void copy_to_string(string * target) const{
        target->assign(m_ptr,m_length);
    }
    bool starts_with(const StringPiece & x) const{
        return ((m_length >= x.m_length) &&(memcmp(m_ptr,x.m_ptr,x.m_length) == 0));
    }
};
std::ostream & operator<<(std::ostream & o,const czy::StringPiece & piece);
}