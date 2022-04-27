#include"asynclog.h"
#include"log_file.h"
#include"time_stamp.h"
#include<stdio.h>
//异步写入文件，两个缓冲区
czy::AsyncLogging::AsyncLogging(const string & basename,off_t roll_size,
int flush_interval):
 m_flush_interval(flush_interval),m_running(false),
 m_basename(basename),
 m_roll_size(roll_size),
 //bind本身是一种延迟计算的思想，它本身可以绑定普通函数、全局函数、静态函数、类静态函数甚至是类成员函数
 m_thread(std::bind(&AsyncLogging::thread_func,this),"Logging"),
//这样之后，类似于将this->thread_func传给MThread
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
    //unique_ptr无法进行拷贝
    //但可以进行移动
    //unique_ptr能够保证对象一定会被析构，即使程序抛出异常
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
            czy::MutexGroud lock (m_mutex);//名字一定要加上，防止编译器自动释放锁
            //区别于传统的cond用法，
            //这里使用if来判断，
            //即超时时直接进行下一步操作
            if(m_buffers.empty()){
                m_cond.wait_for_seconds(m_flush_interval);
            }
            //使用std::move提高效率，而且也能完成想要的交换功能
            m_buffers.push_back(std::move(m_current_buffer));
            m_current_buffer = std::move(new_buffer_ptr1);
            //当m_next_buffer为空，移动第二块缓冲
            if(!m_next_buffer){
                m_next_buffer = std::move(new_buffer_ptr2);
            }
        }//使用花括号来管理MutexGroud的作用空间     
        assert(!m_buffers.empty());
        //一次写入过多的情况
        //需要丢弃并且打印信息，只保留两个Buffer的信息
        if(buffers_to_writes.size() > 25)
        {
            char buf[256];
            snprintf(buf,sizeof(buf),"Dropped log message at %s ,%zd large buffers\n",
                TimeStamp::now().to_formatted_string().c_str(),
                buffers_to_writes.size() -2
            );
            //fputs用于将buf[]里的写入到流中
            fputs(buf,stderr);
            output.append(buf,static_cast<int> (strlen(buf)));
            buffers_to_writes.erase(buffers_to_writes.begin()+2,buffers_to_writes.end());
        }
        //对于unique_ptr的使用必须加上&才能访问
        for(auto& buf:buffers_to_writes){
            output.append(buf->data(),buf->length());
        }
        //
        if(buffers_to_writes.size() >2){
            buffers_to_writes.resize(2);
        }
        if(!new_buffer_ptr1){
            //直接从buffer_to_write 中抽取并且move
            //效率十分高
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