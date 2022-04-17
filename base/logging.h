#pragma once
#include"log_stream.h"
#include"time_stamp.h"
namespace czy{

class TimeZone;
class Logger{
public:
    //enum类型存放日志级别
    enum LogLevel{
        TRACE,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL,
        NUM_LOG_LEVELS
    };
    //嵌套类，其访问范围只在类的内部
    class SourceFile{
    public:
        template<int N>
        SourceFile(const char (&arr)[N]):m_data(arr),m_size(N-1){
            const char *slash = strchr(m_data,'/');
            if(slash){
                m_data = slash+1;
                m_size -= static_cast<int>(m_data - arr);
            }
        }  
        const char * m_data;
        int m_size;
        explicit SourceFile(const char * filename)
         :m_data(filename)
        {
            const char * slash = strchr(filename,'/');
            if(slash){
                m_data = slash + 1;
            }
            m_size = static_cast<int> (strlen(m_data));
        }
    };
  Logger(SourceFile file, int line);
  Logger(SourceFile file, int line, LogLevel level);
  Logger(SourceFile file, int line, LogLevel level, const char* func);
  Logger(SourceFile file, int line, bool toAbort);
  ~Logger();
  LogStream& stream(){ return m_impl.m_stream; }
  static LogLevel log_level();
  static void set_log_level(LogLevel level);
  typedef void (*OutPutFunc)(const char * msg,int len);
  typedef void (*FlushFunc)();
  static void set_out_put(OutPutFunc);
  static void set_flush(FlushFunc);
  static void set_time_zone(const TimeZone &tz);
private:
    class Impl{
    public:
        typedef Logger::LogLevel LogLevel;
        Impl(LogLevel level,int old_errno,const SourceFile &file,int len);
        void format_time();
        void finish();
        TimeStamp m_time;
        LogStream m_stream;
        int m_line;
        SourceFile m_base_name;  
    };
    Impl m_impl;
};
//extern 方便外部访问
extern Logger::LogLevel g_loglevel;
inline Logger::LogLevel Logger::log_level(){
    return g_loglevel;
}
//__FILE__宏::输出源文件文件名,__LINE__输出在源文件行号，FUNC输出函数名
#define LOG_TRACE if(czy::Logger::log_level() <= czy::Logger::TRACE)\
 czy::Logger(__FILE__,__LINE__,czy::Logger::TRACE,__func__).stream()
#define LOG_DEBUG if(czy::Logger::log_level() <= czy::Logger::DEBUG)\
 czy::Logger(__FILE__,__LINE__,czy::Logger::DEBUG,__func__).stream()
#define LOG_INFO if(czy::Logger::log_level() <= czy::Logger::INFO)\
 czy::Logger(__FILE__,__LINE__).stream()
#define LOG_WARN czy::Logger(__FILE__, __LINE__, czy::Logger::WARN).stream()
#define LOG_ERROR czy::Logger(__FILE__, __LINE__, czy::Logger::ERROR).stream()
#define LOG_FATAL czy::Logger(__FILE__, __LINE__, czy::Logger::FATAL).stream()
#define LOG_SYSERR czy::Logger(__FILE__, __LINE__, false).stream()
#define LOG_SYSFATAL czy::Logger(__FILE__, __LINE__, true).stream()
const char *strerror_tl(int saved_errorno);
#define CHECK_NOTNULL(val) \
  ::czy::CheckNotNull(__FILE__, __LINE__, "'" #val "' Must be non NULL", (val))

// A small helper for CHECK_NOTNULL().
template <typename T>
T* CheckNotNull(Logger::SourceFile file, int line, const char *names, T* ptr)
{
  if (ptr == NULL)
  {
   Logger(file, line, Logger::FATAL).stream() << names;
  }
  return ptr;
}
}