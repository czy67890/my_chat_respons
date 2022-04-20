#include"log_file.h"
#include"file_util.h"
#include"process_info.h"
#include<assert.h>
#include<stdio.h>
#include<time.h>
czy::LogFile::LogFile(const string& basename,
          off_t rollSize,
          bool threadSafe = true,
          int flushInterval = 3,
          int checkEveryN = 1024)
:m_basename(basename),m_roll_size(rollSize),
m_flush_interval(flushInterval),m_check_every_n(checkEveryN),
//这里的m_mutex在不需要线程安全的情况下可以直接置NULL,提高线程访问速度
m_count(0),m_mutex(threadSafe ? new Mutex : NULL),m_start_of_period(0),
m_last_roll(0),m_last_flush(0)
{
    assert(basename.find('/') == string::npos);
    roll_file();
}
czy::LogFile::~LogFile() = default;
//一种设计手段,将需要加锁的和不需要加锁的分离开来
//实现加锁和不需要加锁都可以访问
void czy::LogFile::append(const char *logline,int len){
    if(m_mutex){
        //要用*来对unique_ptr进行取值后访问
        MutexGroud lock(*m_mutex);
        append_unlocked(logline,len);
    }
    else{
        append_unlocked(logline,len);
    }
}

void czy::LogFile::flush(){
    if(m_mutex){
        MutexGroud lock(*m_mutex);
        m_file->flush();
    }
    else{
        m_file->flush();
    }
}

void czy::LogFile::append_unlocked(const char * logline,int len){
    m_file->append(logline,len);
    if(m_file->written_bytes() > m_roll_size){
        roll_file();
    }
    else{
        ++m_count;
        if(m_count >= m_check_every_n){
            m_count = 0;
            time_t now = time(NULL);
            //先除上一天的秒数，再乘回去，旨在对齐到当天零点
            time_t m_this_peroid = now/k_roll_per_seconds*k_roll_per_seconds;
            if(m_this_peroid != m_start_of_period){
                roll_file();
            }
            else if(now - m_last_flush > m_flush_interval){
                m_last_flush = now;
                m_file->flush();
            }
        }
    }
}
bool czy::LogFile::roll_file(){
    time_t now = 0;
    string file_name = get_log_filename(m_basename,&now);
    time_t start = now/k_roll_per_seconds *k_roll_per_seconds;
    if(now > m_last_roll){
        m_last_roll = now;
        m_last_flush = now;
        m_start_of_period = start;
        m_file.reset(new FileUtil::AppendFile(file_name));
        return true;
    }
    return false;
}

std::string czy::LogFile::get_log_filename(const string & basename, time_t* now){
    string file_name;
    //reserve函数，只是改变capacity并不会创建对象，也不会改变大小
    //resize函数，即创建对象，也改变大小，也改变size
    file_name.reserve(basename.size() + 64);
    file_name = basename;
    char time_buf[32];
    struct tm tm;
    *now = time(NULL);
    //gmt用来获取标准时间
    gmtime_r(now,&tm);
    //strftime用于格式化时间转换成字符串
    strftime(time_buf,sizeof(time_buf),".%Y%m%d-%H%M%S",&tm);
    file_name += time_buf;
    file_name += czy::ProcessInfo::hostname();
    char pidbuf[32];
    snprintf(pidbuf,sizeof(pidbuf),".%d",ProcessInfo::pid());
    file_name += pidbuf;
    file_name += ".log";
    return file_name;
}