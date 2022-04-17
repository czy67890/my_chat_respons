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

  std::unique_ptr<Mutex> mutex_;
  time_t m_start_of_period;
  time_t m_last_roll;
  time_t m_last_flush;
  std::unique_ptr<FileUtil::AppendFile> m_file;

  const static int k_roll_per_seconds = 60*60*24;
};

} 