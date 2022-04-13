#pragma once
#include<boost/operators.hpp>
namespace czy{
class TimeStamp :public boost::equality_comparable<TimeStamp>,
                public boost::less_than_comparable<TimeStamp>{
public:
    TimeStamp() :m_micro_since_epoch(0){
    }
    //声明为explict的防止隐式转换
    explicit TimeStamp(int64_t micro_since_epoch):m_micro_since_epoch(micro_since_epoch){
    }

private:
    int64_t m_micro_since_epoch;
};
}