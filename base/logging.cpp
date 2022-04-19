#include"logging.h"
#include"current_thread.h"
#include<errno.h>
#include<stdio.h>
#include<string.h>
#include<sstream>
#include"time_stamp.h"
#include"time_zone.h"
namespace czy{
//__thread 线程自己拥有的变量
__thread char t_errbuf[512];
__thread char t_time[64];
__thread time_t t_last_second;
const char * strerr_tl(int saved_errno){
    return strerror_r(saved_errno,t_errbuf,sizeof(t_errbuf));
}
Logger::LogLevel init_log_level(){
    //getenv()为获取当前环境变量的函数，返回NULL说明该环境变量不存在
    if(::getenv("CZY_LOG_TRACE"))
        return Logger::TRACE;
    else if(::getenv("CZY_LOG_DEBUG")){
        return Logger::DEBUG;
    } 
    else{
        return Logger::INFO;
    }
}
Logger::LogLevel g_loglevel = init_log_level();
const char * log_m_levelname[Logger::NUM_LOG_LEVELS] = {
    "TRACE",
    "DEBUG",
    "INFO",
    "WARN",
    "ERROR",
    "FATAL",
};
//help class for kown string length at compile time
class T{
public:
  T(const char * str,unsigned len):
   m_str(str),
   m_len(len){
       assert(strlen(str) == m_len);
   } 
const char *m_str;
const unsigned m_len;
};

inline LogStream & operator<<(LogStream &s ,T v){
    s.append(v.m_str,v.m_len);
    return s;
}

inline LogStream & operator<<(LogStream &s, const Logger::SourceFile v){
    s.append(v.m_data,v.m_size);
    return s;
}

void default_output(const char * msg,int len){
    size_t n = fwrite(msg,1,len,stdout);
    (void) n;
} 

void default_flush(){
    fflush(stdout);
}

Logger::OutPutFunc g_output = default_output;
Logger::FlushFunc g_flush = default_flush;
TimeZone g_log_time_zone;
}

using namespace czy;
Logger::Impl::Impl(LogLevel level,int saved_no,const SourceFile &file,int line)
 :m_time(TimeStamp::now()),m_stream(),m_level(level)
 ,m_base_name(file){
     format_time();
     CurrentThread::tid();
     m_stream<<T(CurrentThread::tid_string(),CurrentThread::tid_length());
     m_stream<<T(log_m_levelname[level],6);
     if(saved_no != 0){
         m_stream<<strerr_tl(saved_no)<<"(errno = "<<saved_no<<")";
     }
 }

 void Logger::Impl::format_time(){
     int64_t micro_seconds_since = m_time.micro_since_epoch();
     time_t seconds = static_cast<time_t> (micro_seconds_since/TimeStamp::k_micro_seconds_persecond);
     int micro_seconds = static_cast<int> (micro_seconds_since%TimeStamp::k_micro_seconds_persecond);
     if(seconds != t_last_second){
        t_last_second = seconds;
        struct tm tm_time;
        if(g_log_time_zone.valid()){
            tm_time = g_log_time_zone.to_local_time(seconds);
        }
        else{
            ::gmtime_r(&seconds,&tm_time);
        }
        int len = snprintf(t_time,sizeof(t_time),"%4d%02d%02d %02d:%02d:%02d",
            tm_time.tm_year + 1900,tm_time.tm_mon +1 ,tm_time.tm_mday,
            tm_time.tm_hour,tm_time.tm_min,tm_time.tm_sec
        );
        assert(len == 17);
        (void) len;
     }
     if(g_log_time_zone.valid()){
         Fmt us(".%06d ",micro_seconds);
         assert(us.length() == 8);
         m_stream << T(t_time,17)<<T(us.data(),8);
     }
     else{
         Fmt us("%06dZ ",micro_seconds);
         assert(us.length() == 9);
         m_stream<<T(t_time,17)<<T(us.data(),9);
     }
 }
 
void Logger::Impl::finish()
{
  m_stream << " - " << m_base_name << ':' << m_line << '\n';
}

Logger::Logger(SourceFile file, int line)
  : m_impl(INFO, 0, file, line)
{
}

Logger::Logger(SourceFile file, int line, LogLevel level, const char* func)
  : m_impl(level, 0, file, line)
{
  m_impl.m_stream << func << ' ';
}

Logger::Logger(SourceFile file, int line, LogLevel level)
  : m_impl(level, 0, file, line)
{
}

Logger::Logger(SourceFile file, int line, bool toAbort)
  : m_impl(toAbort?FATAL:ERROR, errno, file, line)
{
}

Logger::~Logger()
{
  m_impl.finish();
  const LogStream::Buffer& buf(stream().buffer());
  g_output(buf.data(), buf.length());
  if (m_impl.m_level == FATAL)
  {
    g_flush();
    abort();
  }
}

void Logger::set_log_level(Logger::LogLevel level)
{
  g_loglevel = level;
}

void Logger::set_out_put(OutPutFunc out)
{
  g_output = out;
}

void Logger::set_flush(FlushFunc flush)
{
  g_flush = flush;
}

void Logger::set_time_zone(const TimeZone& tz)
{
  g_log_time_zone = tz;
}

