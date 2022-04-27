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
 m_latch(1),m_mutex(),m_cond(m_mutex),m_current_buffer(new Buffer),
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
    //unique_ptr�޷����п���
    //�����Խ����ƶ�
    //unique_ptr�ܹ���֤����һ���ᱻ��������ʹ�����׳��쳣
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
            //�����ڴ�ͳ��cond�÷���
            //����ʹ��if���жϣ�
            //����ʱʱֱ�ӽ�����һ������
            if(m_buffers.empty()){
                m_cond.wait_for_seconds(m_flush_interval);
            }
            //ʹ��std::move���Ч�ʣ�����Ҳ�������Ҫ�Ľ�������
            m_buffers.push_back(std::move(m_current_buffer));
            m_current_buffer = std::move(new_buffer_ptr1);
            //��m_next_bufferΪ�գ��ƶ��ڶ��黺��
            if(!m_next_buffer){
                m_next_buffer = std::move(new_buffer_ptr2);
            }
        }//ʹ�û�����������MutexGroud�����ÿռ�     
        assert(!m_buffers.empty());
        //һ��д���������
        //��Ҫ�������Ҵ�ӡ��Ϣ��ֻ��������Buffer����Ϣ
        if(buffers_to_writes.size() > 25)
        {
            char buf[256];
            snprintf(buf,sizeof(buf),"Dropped log message at %s ,%zd large buffers\n",
                TimeStamp::now().to_formatted_string().c_str(),
                buffers_to_writes.size() -2
            );
            //fputs���ڽ�buf[]���д�뵽����
            fputs(buf,stderr);
            output.append(buf,static_cast<int> (strlen(buf)));
            buffers_to_writes.erase(buffers_to_writes.begin()+2,buffers_to_writes.end());
        }
        //����unique_ptr��ʹ�ñ������&���ܷ���
        for(auto& buf:buffers_to_writes){
            output.append(buf->data(),buf->length());
        }
        //
        if(buffers_to_writes.size() >2){
            buffers_to_writes.resize(2);
        }
        if(!new_buffer_ptr1){
            //ֱ�Ӵ�buffer_to_write �г�ȡ����move
            //Ч��ʮ�ָ�
            assert(!buffers_to_writes.empty());
            new_buffer_ptr1 = std::move(buffers_to_writes.back());
            buffers_to_writes.pop_back();
            new_buffer_ptr1.reset();
        }
        if(!new_buffer_ptr2){
            assert(!buffers_to_writes.empty());
            new_buffer_ptr2 = std::move(buffers_to_writes.back());
            buffers_to_writes.pop_back();
            new_buffer_ptr2.reset();
        }
        buffers_to_writes.clear();
        output.flush();
    }   
}