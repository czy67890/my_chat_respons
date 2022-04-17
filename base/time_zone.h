#pragma once
#include<memory>
#include<time.h>
namespace czy
{
class TimeZone{
public:
    explicit TimeZone(const char *zone_file);
    TimeZone(int east_of_utc,const char * zone_file);
    TimeZone() = default;
    bool vaild()const
    {
        return static_cast<bool> (m_data);
    }
    struct tm to_local_time(time_t seconds_since_epoch) const;
    time_t from_local_time(const struct tmt &) const;
    static struct tm to_utc_time(time_t second_since_epoch,bool yday = false);
    static time_t from_utc_time(const struct tm&);
    static time_t from_utc_time(int year,int month,int day,int hour,int minute,int seconds);
    struct Data;
private:
    std::shared_ptr<Data> m_data;
};
}