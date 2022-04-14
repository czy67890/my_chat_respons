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
    void swap(TimeStamp & that){
        std::swap(m_micro_since_epoch,that.m_micro_since_epoch);
    }
    std::string to_string() const;
    std::string to_formatted_string(bool show_micro_second_since_epoch = true) const;
    bool valid() const {return m_micro_since_epoch > 0;}
    int64_t micro_since_epoch(){return m_micro_since_epoch;}
    time_t seconds_since_epoch(){
        return static_cast<time_t>(m_micro_since_epoch/k_micro_seconds_persecond);
    }
    static TimeStamp now();
    static TimeStamp invalid(){
        return TimeStamp();
    }
    static TimeStamp from_unix_time(time_t t,int microseconds){
        return TimeStamp(static_cast<int64_t> (t)* k_micro_seconds_persecond);
    }
    static const int k_micro_seconds_persecond = 1000 * 1000;
private:
    int64_t m_micro_since_epoch;
};
inline bool operator<(TimeStamp lhs,TimeStamp rhs){
    return lhs.micro_since_epoch()<rhs.micro_since_epoch();
}
inline bool operator==(TimeStamp lhs,TimeStamp rhs){
    return lhs.micro_since_epoch()==rhs.micro_since_epoch();
}
inline double time_diffrence(TimeStamp high,TimeStamp low){
    int64_t diff = high.micro_since_epoch() - low.micro_since_epoch();
    return static_cast<double> (diff) / TimeStamp::k_micro_seconds_persecond;
}
inline TimeStamp add_time(TimeStamp time_stamp,double seconds){
    int64_t delta = static_cast<int64_t>(seconds *TimeStamp::k_micro_seconds_persecond);
    return TimeStamp(time_stamp.micro_since_epoch() + delta);
}
}  