#pragma once
#include"locker.h"
#include"nocopyable.h"
#include<deque>
#include<assert.h>
//无界锁
namespace czy{
template<typename T>
class BlockingQueue: nocopyable{
public:
    using queue_type = std::deque<T>;
    BlockingQueue():m_mutex(),m_not_empty(m_mutex),m_queue(){};
    void put(const T &x){
        MutexGroud g(m_mutex);
        m_queue.push_back(x);
        m_not_empty.post();
    }
    void put(const T&& x){
        MutexGroud g(m_mutex);
        //右值情况下需要引用
        m_queue.push_back(move(x));
        m_not_empty.post();
    }
    T take(){
        MutexGroud(m_mutex);
        while(m_queue.empty()){
            m_not_empty.wait();
        }
        assert(!m_queue.empty());
        T front(std::move(m_queue.front()));
        m_queue.pop_front();
        return front;
    }
    queue_type drain(){
        std::deque<T> queue;
        //用花括号手动控制临界区的大小
        {
            MutexGroud(m_mutex);
            queue = std::move(m_queue);
            assert(m_queue.empty());
        }
        return queue;
    }
    size_t size() const{
        MutexGroud(m_mutex);
        return m_queue.size();
    }
private:
    mutable Mutex m_mutex;
    Cond          m_not_empty;
    queue_type    m_queue;
};
}