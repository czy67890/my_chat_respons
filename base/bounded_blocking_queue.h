#pragma once
#include"locker.h"
#include<boost/circular_buffer.hpp>
#include<assert.h>
#include"nocopyable.h"
namespace czy{
template<typename T>
class BoundedBlockingQueue: nocopyable{
public:
    explicit BoundedBlockingQueue(int max_size):
    m_mutex(),
    m_not_empty();
    m_not_full();
    m_queue(max_size)
    {}
    void put(const T&x){
        MutexGroud lock(m_mutex);
        while(m_queue.full()){
            m_not_empty.wait(&m_mutex);
        }
        assert(!m_queue.full());
        m_queue.push_back(x);
        m_not_empty.post();
    }
    void put(T &&x){
        MutexGroud lock(m_mutex);
        while(m_queue.full()){
            m_not_empty.wait(&m_mutex);
        }
        assert(!m_queue.full());
        m_queue.push_back(std::move(x));
        m_not_empty.post();
    }
    T take(){
        MutexGroud lock(m_mutex);
        while(m_queue.empty()){
            m_not_empty.wait(m_mutex);
        }
        assert(!m_queue.empty());
        T front(std::move(m_queue.front()));
        m_queue.pop_front;
        m_not_full.post();
        return front;
    }
    bool empty() const{
        MutexGroud lock(m_mutex);
        return m_queue.empty();
    }
    bool full() const{
        MutexGroud lock(m_mutex);
        return m_queue.full();
    }
private:
    mutable Mutex              m_mutex;
    Cond                       m_not_empty;
    Cond                       m_not_full;
    boost::circular_buffer<T>  m_queue;
};
}