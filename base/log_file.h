#pragma once
#include"locker.h"
#include"types.h"
#include<memory>
#include"nocopyable.h"
namespace czy
{

namespace FileUtil
{
class AppendFile;
}

class LogFile : nocopyable
{
 public:
  LogFile(const string& basename,
          off_t rollSize,
          bool threadSafe = true,
          int flushInterval = 3,
          int checkEveryN = 1024);
  ~LogFile();

  void append(const char* logline, int len);
  void flush();
  bool roll_file();

 private:
  void append_unlocked(const char* logline, int len);

  static string get_log_filename(const string& basename, time_t* now);

  const string m_basename;
  const off_t m_roll_size;
  const int m_flush_interval;
  const int m_check_every_n;

  int m_count;

  std::unique_ptr<Mutex> m_mutex;
  time_t m_start_of_period;
  time_t m_last_roll;
  time_t m_last_flush;
  //unique_ptr用于独占的享有Mutex，没有拷贝构造和拷贝赋值的操作
  //只能通过move将一个右值转换成左值来进行转移
  std::unique_ptr<FileUtil::AppendFile> m_file;
  const static int k_roll_per_seconds = 60*60*24;
};

} 