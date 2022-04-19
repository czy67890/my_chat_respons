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
            
        }
    }
}
