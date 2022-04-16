#include"log_stream.h"
#include<algorithm>
#include<limits>
#include<type_traits>
#include<assert.h>
#include<string.h>
#include<stdio.h>
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#include<inttypes.h>
using namespace czy;
using namespace czy::detail;
#pragma GCC diagnostic ignored "-Wtype-limits" 
namespace czy{
namespace detail{


const char digits[] = "9876543210123456789";
const char * zero = digits+9;
static_assert(sizeof(digits) == 20,"wrong number of digit");
const char digitsHex[] = "0123456789ABCDEF";
static_assert(sizeof(digitsHex) == 17,"wrong number of digitHex");
template<typename T>
size_t convert(char buf[],T value){
    T i = value;
    char * p = buf;
    do
    { 
        *p++ = zero[static_cast<int>(i%10)];
        i/=10;
    }while(i != 0);
    if(value < 0){
        *p++ = '-';
    }
    *p = '\0';
    std::reverse(buf,p);
    return p - buf;
}
size_t convert_hex(char buf[],uintptr_t value){
    uintptr_t i = value;
    char *p = buf;
    do
    {
        int lsd = static_cast<int>(i % 16);
        i /= 16;
        *p++ = digitsHex[lsd];
    }while(i != 0);
    std::reverse(buf,p);
    return p - buf;
}
template class FixBuffer<k_small_buffer>;
template class FixBuffer<k_large_buffer>;
}//namespace detail

std::string formatSI(int64_t s){
    double n = static_cast<double>(s);
  char buf[64];
  if (s < 1000)
    snprintf(buf, sizeof(buf), "%" PRId64, s);
  else if (s < 9995)
    snprintf(buf, sizeof(buf), "%.2fk", n/1e3);
  else if (s < 99950)
    snprintf(buf, sizeof(buf), "%.1fk", n/1e3);
  else if (s < 999500)
    snprintf(buf, sizeof(buf), "%.0fk", n/1e3);
  else if (s < 9995000)
    snprintf(buf, sizeof(buf), "%.2fM", n/1e6);
  else if (s < 99950000)
    snprintf(buf, sizeof(buf), "%.1fM", n/1e6);
  else if (s < 999500000)
    snprintf(buf, sizeof(buf), "%.0fM", n/1e6);
  else if (s < 9995000000)
    snprintf(buf, sizeof(buf), "%.2fG", n/1e9);
  else if (s < 99950000000)
    snprintf(buf, sizeof(buf), "%.1fG", n/1e9);
  else if (s < 999500000000)
    snprintf(buf, sizeof(buf), "%.0fG", n/1e9);
  else if (s < 9995000000000)
    snprintf(buf, sizeof(buf), "%.2fT", n/1e12);
  else if (s < 99950000000000)
    snprintf(buf, sizeof(buf), "%.1fT", n/1e12);
  else if (s < 999500000000000)
    snprintf(buf, sizeof(buf), "%.0fT", n/1e12);
  else if (s < 9995000000000000)
    snprintf(buf, sizeof(buf), "%.2fP", n/1e15);
  else if (s < 99950000000000000)
    snprintf(buf, sizeof(buf), "%.1fP", n/1e15);
  else if (s < 999500000000000000)
    snprintf(buf, sizeof(buf), "%.0fP", n/1e15);
  else
    snprintf(buf, sizeof(buf), "%.2fE", n/1e18);
  return buf;
}

std::string formatIEC(int64_t s){
    double n = static_cast<double>(s);
  char buf[64];
  const double Ki = 1024.0;
  const double Mi = Ki * 1024.0;
  const double Gi = Mi * 1024.0;
  const double Ti = Gi * 1024.0;
  const double Pi = Ti * 1024.0;
  const double Ei = Pi * 1024.0;

  if (n < Ki)
    snprintf(buf, sizeof buf, "%" PRId64, s);
  else if (n < Ki*9.995)
    snprintf(buf, sizeof buf, "%.2fKi", n / Ki);
  else if (n < Ki*99.95)
    snprintf(buf, sizeof buf, "%.1fKi", n / Ki);
  else if (n < Ki*1023.5)
    snprintf(buf, sizeof buf, "%.0fKi", n / Ki);

  else if (n < Mi*9.995)
    snprintf(buf, sizeof buf, "%.2fMi", n / Mi);
  else if (n < Mi*99.95)
    snprintf(buf, sizeof buf, "%.1fMi", n / Mi);
  else if (n < Mi*1023.5)
    snprintf(buf, sizeof buf, "%.0fMi", n / Mi);

  else if (n < Gi*9.995)
    snprintf(buf, sizeof buf, "%.2fGi", n / Gi);
  else if (n < Gi*99.95)
    snprintf(buf, sizeof buf, "%.1fGi", n / Gi);
  else if (n < Gi*1023.5)
    snprintf(buf, sizeof buf, "%.0fGi", n / Gi);

  else if (n < Ti*9.995)
    snprintf(buf, sizeof buf, "%.2fTi", n / Ti);
  else if (n < Ti*99.95)
    snprintf(buf, sizeof buf, "%.1fTi", n / Ti);
  else if (n < Ti*1023.5)
    snprintf(buf, sizeof buf, "%.0fTi", n / Ti);

  else if (n < Pi*9.995)
    snprintf(buf, sizeof buf, "%.2fPi", n / Pi);
  else if (n < Pi*99.95)
    snprintf(buf, sizeof buf, "%.1fPi", n / Pi);
  else if (n < Pi*1023.5)
    snprintf(buf, sizeof buf, "%.0fPi", n / Pi);

  else if (n < Ei*9.995)
    snprintf(buf, sizeof buf, "%.2fEi", n / Ei );
  else
    snprintf(buf, sizeof buf, "%.1fEi", n / Ei );
  return buf;
}

}//namespace czy
template<int SIZE>
const char * FixBuffer<SIZE>::debug_string(){
  *m_cur = '\0';
  return m_data;
}
template<int SIZE>
void FixBuffer<SIZE>::cookie_start(){

} 
template<int SIZE>
void FixBuffer<SIZE>::cookie_end(){

}

//
void LogStream::static_check()
{
  static_assert(k_max_num_size - 10 > std::numeric_limits<double>::digits10
  ,"k_max_num_size is large enough");
  static_assert(k_max_num_size - 10 > std::numeric_limits<long double>::digits10
  ,"k_max_num_size is large enough");
  static_assert(k_max_num_size - 10 > std::numeric_limits<long>::digits10
  ,"k_max_num_size is large enough");
  static_assert(k_max_num_size - 10 > std::numeric_limits<long long>::digits10
  ,"k_max_num_size is large enough");
}
template<typename T>
void LogStream::format_integer(T v){
  if(m_buffer.avail() >= k_max_num_size){
    size_t len = convert(m_buffer.current());
    m_buffer.add(len);
  } 
}
//重载LogStream的<<方法
LogStream& LogStream::operator<<(short v){
  *this << static_cast<int>(v);
  return *this;
}

LogStream& LogStream::operator<<(unsigned short v){
  *this << static_cast<unsigned int> (v);
  return *this;
}

LogStream& LogStream::operator<<(int v){
  format_integer(v);
  return *this;
}

LogStream& LogStream::operator<<(unsigned int v){
  format_integer(v);
  return *this;
}

LogStream& LogStream::operator<<(unsigned long v){
  format_integer(v);
  return *this;
}

LogStream& LogStream::operator<<(unsigned long long v){
  format_integer(v);
  return *this;
}

LogStream& LogStream::operator<<(const void * p){
  //uintptr_t 类型用于增强代码的可移植性
  //在32位机器上为unsigned int 
  //在64位机器上位unsigned long int
  uintptr_t v = reinterpret_cast<uintptr_t> (p);
  if(m_buffer.avail() >= k_max_num_size){
    char * buf = m_buffer.current();
    buf[0] = '0';
    buf[1] = 'x';
    size_t len = convert_hex(buf+2,v);
    m_buffer.add(len+2);
    return *this;
  }
}

LogStream& LogStream::operator<<(double v){
  if(m_buffer.avail() >= k_max_num_size){
    int len = snprintf(m_buffer.current(),k_max_num_size,"%.12g",v);
    m_buffer.add(len); 
  }
  return *this;
}

template<typename T>
Fmt::Fmt(const char * fmt,T val){
  //判断该值是否为类型值(即非类与非struct对象)
  static_assert(std::is_arithmetic<T>::value == true,
  "Must be arithmetic type");
  m_length = snprintf(m_buf,sizeof(m_buf),m_fmt,val);
  assert(static_cast<size_t>(m_length) < sizeof(m_buf));
}
//将上面的模板类特列化
template Fmt::Fmt(const char * fmt,char);
template Fmt::Fmt(const char * fmt,short);
template Fmt::Fmt(const char * fmt,unsigned short);
template Fmt::Fmt(const char * fmt,long);
template Fmt::Fmt(const char * fmt,unsigned long);
template Fmt::Fmt(const char * fmt,long long);
template Fmt::Fmt(const char * fmt,unsigned long long);
template Fmt::Fmt(const char * fmt,float);
template Fmt::Fmt(const char * fmt,double);
