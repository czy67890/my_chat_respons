#include"asynclog.h"
#include"log_file.h"
#include"time_stamp.h"
#include<stdio.h>
//�첽д���ļ�������������
czy::AsyncLogging::AsyncLogging(const string & basename,off_t roll_size,
int flush_interval):
 m_flush_interval(flush_interval),m_running(false),
 m_basename(basename),
 m_roll_size(roll_size),
 //bind������һ���ӳټ����˼�룬��������԰���ͨ������ȫ�ֺ�������̬�������ྲ̬�������������Ա����
 m_thread(std::bind(&AsyncLogging::thread_func,this),"Logging"),
//����֮�������ڽ�this->thread_func����MThread
 m_latch(1),m_mutex(),m_cond(),m_current_buffer(new Buffer),
 m_next_buffer(new Buffer),
 m_buffers()
{
    m_current_buffer->bzero();
    m_next_buffer->bzero();
    m_buffers.reserve(16);
}

void czy::AsyncLogging::append(const char * logline,int len){
    czy::MutexGroud lock(m_mutex);
    if(m_current_buffer->avail() > len){
        m_current_buffer->append(logline,len);
    }
    else{
        m_buffers.push_back(std::move(m_current_buffer));
        if(m_next_buffer){
            m_current_buffer = std::move(m_next_buffer);
        }
        else{
            m_current_buffer.reset(new Buffer);
        }
        m_current_buffer->append(logline,len);
        m_cond.post();
    }
}

void czy::AsyncLogging::thread_func(){
    assert(m_running == true);
    m_latch.count_down();
    czy::LogFile output(m_basename,m_roll_size,false);
    BufferPtr new_buffer_ptr1(new Buffer);
    BufferPtr new_buffer_ptr2(new Buffer);
    new_buffer_ptr1->bzero();
    new_buffer_ptr2->bzero();
    BufferVector buffers_to_writes;
    buffers_to_writes.reserve(6);
    while(m_running){
        assert(new_buffer_ptr1 && new_buffer_ptr1->length()== 0);
        assert(new_buffer_ptr1 && new_buffer_ptr1->length()== 0);
        assert(buffers_to_writes.empty());
        {
            czy::MutexGroud lock (m_mutex);//����һ��Ҫ���ϣ���ֹ�������Զ��ͷ���
            if(m_buffers.empty()){
                m_cond.
            }
        }//ʹ�û�����������MutexGroud�����ÿռ�     
    }   
}