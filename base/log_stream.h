#pragma once
#include"nocopyable.h"
#include"types.h"
#include"types.h"
#include<assert.h>
#include"string_piece.h"
namespace czy{
namespace detail{

const int k_small_buffer = 4000;
const int k_large_buffer = 4000*1000;
template<int SIZE>
class FixBuffer : nocopyable{
public:
    FixBuffer()
     :m_cur(m_data){
         set_cookie(cookie_start);
     }
    ~FixBuffer(){
        set_cookie(cookie_end);
    }
    char *current(){ return m_cur; }
    void append(const char * buf,size_t len){
        //implicit_cast 实现安全的上转型
        if(implicit_cast<size_t>(avail()) > len)
        {
            memcpy(m_cur,buf,len);
            m_cur += len;
        }
    }
    void set_cookie(void (* cookie)() ){
        m_cookie = cookie;
    }
    const char *data() const{ return m_data;}
    int length() const{ return static_cast<int>(m_cur - m_data); }
    int avail() const{ return static_cast<int>(end() - m_cur);}
    void add(size_t len){
        m_cur += len;
    }
    void reset(){ m_cur = m_data; }
    void bzero(){
        mem_zero(m_data,sizeof(m_data));
    }
    const char * debug_string();
    string to_string() const{
        return string(m_cur,length());
    }
    StringPiece to_string_piece() const{
        return StringPiece(m_data,length());
    }
private:
    static void cookie_start();
    static void cookie_end(); 
    void( * m_cookie)();
    const char * end() const {return m_data + sizeof(m_data);}
    char m_data[SIZE];
    char *m_cur;
};
}//namespace detail
class LogStream: nocopyable{
public:
typedef LogStream self;
typedef czy::detail::FixBuffer<detail::k_small_buffer> Buffer;
public: 
    //流使用对自己的引用
    self & operator<<( bool v){
        m_buffer.append(v?"1":"0",1);
        return *this;
    }
    self &operator<<(short);
    self &operator<<(unsigned short);
    self &operator<<(int);
    self &operator<<(unsigned int);
    self &operator<<(long);
    self &operator<<(unsigned long);
    self &operator<<(long long);
    self &operator<<(unsigned long long);
      self& operator<<(const void*);

  self& operator<<(float v)
  {
    *this << static_cast<double>(v);
    return *this;
  }
  self& operator<<(double);
  // self& operator<<(long double);

  self& operator<<(char v)
  {
    m_buffer.append(&v, 1);
    return *this;
  }

  // self& operator<<(signed char);
  // self& operator<<(unsigned char);

  self& operator<<(const char* str)
  {
    if (str)
    {
      m_buffer.append(str, strlen(str));
    }
    else
    {
      m_buffer.append("(null)", 6);
    }
    return *this;
  }

  self& operator<<(const unsigned char* str)
  {
    return operator<<(reinterpret_cast<const char*>(str));
  }

  self& operator<<(const string& v)
  {
    m_buffer.append(v.c_str(), v.size());
    return *this;
  }

  self& operator<<(const StringPiece& v)
  {
    m_buffer.append(v.data(), v.size());
    return *this;
  }

  self& operator<<(const Buffer& v)
  {
    *this << v.to_string_piece();
    return *this;
  }
  void append(const char * data,int len){ m_buffer.append(data,len);}
  const Buffer &buffer() const {return m_buffer;}
  void reset_buffer(){ m_buffer.reset() ;}
private:
    void static_check();
    template<typename T>
    void format_integer(T);
    Buffer m_buffer;
    static const int k_max_num_size = 48;
};
class Fmt{
public:
    template<typename T>
    Fmt(const char * fmt, T val);
    const char * data(){return m_buf;}
    int length() const {return m_length;}
private:
    char m_buf[32];
    int m_length;
};
inline LogStream & operator<<(LogStream & s,Fmt & fmt){
    s.append(fmt.data(),fmt.length());
    return s;
}
string formatSI(int64_t n);
string formatIEC(int64_t n);
}