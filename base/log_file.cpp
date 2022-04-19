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
//�����m_mutex�ڲ���Ҫ�̰߳�ȫ������¿���ֱ����NULL,����̷߳����ٶ�
m_count(0),m_mutex(threadSafe ? new Mutex : NULL),m_start_of_period(0),
m_last_roll(0),m_last_flush(0)
{
    assert(basename.find('/') == string::npos);
    roll_file();
}
czy::LogFile::~LogFile() = default;
//һ������ֶ�,����Ҫ�����ĺͲ���Ҫ�����ķ��뿪��
//ʵ�ּ����Ͳ���Ҫ���������Է���
void czy::LogFile::append(const char *logline,int len){
    if(m_mutex){
        //Ҫ��*����unique_ptr����ȡֵ�����
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
