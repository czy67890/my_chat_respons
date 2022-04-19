#pragma once
#include"types.h"
struct tm;
namespace czy
{
class Date{
    public:
    struct YearMonthDay{
        int year;
        int month;
        int day;
    };
    static const int key_day_per_week = 7;
    static const int k_julian_day_of_1970_01_01;
    Date():m_julian_day_number(0)
    {}
    Date(int year,int month,int day);
    explicit Date(int day)
     :m_julian_day_number(day)
    {}
    explicit Date(const struct tm &);
    //swap仅仅交换地址，所以要用引用
    void swap(Date & that){
        std::swap(m_julian_day_number,that.m_julian_day_number);
    }
    bool valid() const{
        return m_julian_day_number > 0;
    }
    std::string to_iso_string() const;
    struct YearMonthDay yearMonthDay() const;
    int year() const{
        return yearMonthDay().year();
    }
    int month() const{
        return yearMonthDay().month;
    }
    int day() const{
        return yearMonthDay().day;
    }
    int week_day() const{
        return (m_julian_day_number+1)%7;
    }
    int julian_day_number() const{
        return m_julian_day_number;
    }
private:
    int m_julian_day_number;
};

inline bool operator<(Date x,Date y){
        return x.julian_day_number()<y.julian_day_number();
}
inline bool operator == (Date x,Date y){
        return x.julian_day_number() == y.julian_day_number();
}
}