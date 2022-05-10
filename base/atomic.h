#pragma once
#include "nocopyable.h"
#include<stdint.h>
//命名空间，防止造成命名污染
//无锁整数类原子操作，提高效率，避免加锁，
namespace czy{

namespace detail{
template<typename T>
class AtomicIntegerT :nocopyable{
public:
    AtomicIntegerT():value_(0){};
    T get(){
        return __sync_val_compare_and_swap(&value_,0,0);
    }
    T get_and_add(T x){
        return __sync_fetch_and_add(&value_,x);
    }
    T add_and_get(T x){
        return get_and_add(x) + x;
    }
    T increment_and_get(){
        return add_and_get(1);
    }
    T decrement_and_get(){
        return add_and_get(-1);
    }
    T get_and_set(T new_value){
        return __sync_lock_test_and_set(&value_,new_value);
    }
private:
    T value_;
};

}
typedef czy::detail::AtomicIntegerT<int32_t> AtomicInt32;
typedef czy::detail::AtomicIntegerT<int64_t> AtomicInt64;
}