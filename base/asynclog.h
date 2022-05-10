#pragma once
#include"config_file_reader.h"
#include "locker.h"
#include "m_thread.h"
#include"count_down_latch.h"
#include"blocking_queue.h"
#include"bounded_blocking_queue.h"
#include"log_stream.h"
#include<atomic>
#include<vector>
namespace czy{
class AsyncLogging: nocopyable{
public:
    AsyncLogging(const string &basename,off_t roll_size,int flush_int_val = 3);
    ~AsyncLogging(){
        if(m_running){
            stop();
        }
    }
    void append(const char * log_line,int len);
    void start(){
        m_running = true;
        m_thread.start();
        m_latch.wait();
    }
    void stop(){
        m_running = false;
        m_cond.post();
        m_thread.jion();
    }
private:
    void thread_func();
    typedef czy::detail::FixBuffer<czy::detail::k_large_buffer> Buffer;
    typedef std::vector<std::unique_ptr<Buffer>> BufferVector;
    //value_type为每个STL容器类中都有的类型,能够获取到STL容器中存放的类型T
    typedef BufferVector::value_type BufferPtr;
    const int m_flush_interval;
    //c++中的原子类型atomic能够保证多线程访问
    std::atomic<bool> m_running;
    const string m_basename;
    const off_t m_roll_size;
    czy::MThread m_thread;
    czy::CountDownLatch m_latch;
    czy::Mutex     m_mutex;
    czy::Cond      m_cond;
    BufferPtr      m_current_buffer;
    BufferPtr      m_next_buffer;
    BufferVector   m_buffers;
};
}
