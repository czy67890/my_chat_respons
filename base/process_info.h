#pragma once
#include"string_piece.h"
#include"types.h"
#include"time_stamp.h"
#include<vector>
#include<sys/types.h>
namespace czy{

namespace ProcessInfo{
pid_t pid();
  string pid_string();
  uid_t uid();
  string user_name();
  uid_t euid();
  TimeStamp start_time();
  int clock_tick_per_second();
  int pageSize();
  bool isDebugBuild();  // constexpr

  string hostname();
  string procname();
  czy::StringPiece procname(const string& stat);
  /// read /proc/self/status
  string proc_status();

  /// read /proc/self/stat
  string proc_stat();

  /// read /proc/self/task/tid/stat
  string thread_stat();

  /// readlink /proc/self/exe
  string exe_path();

  int open_files();
  int max_open_files();

  struct CpuTime
  {
    double userSeconds;
    double systemSeconds;

    CpuTime() : userSeconds(0.0), systemSeconds(0.0) { }

    double total() const { return userSeconds + systemSeconds; }
  };
  CpuTime cpuTime();

  int num_threads();
  std::vector<pid_t> threads();
}
}