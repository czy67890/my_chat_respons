#include"time_stamp.h"
#include<sys/time.h>
#include<stdio.h>
#include<inttypes.h>
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
using namespace czy;
static_assert(sizeof(TimeStamp) == sizeof(int64_t),"Time has to be int64_t");
std::string TimeStamp::to_string() const{
    char buf[32] = {0};
    int64_t seconds = m_micro_since_epoch/k_micro_seconds_persecond;
    int64_t microseconds = m_micro_since_epoch% k_micro_seconds_persecond;
    //Prid64一种跨平台的64位整数书写方式
    snprintf(buf,sizeof(buf),"%" PRId64 ".%06" PRId64 "",seconds,microseconds);
    return buf;
}
std::string  TimeStamp::to_formatted_string(bool show_micro) const{
    char buf[64] = {0};
    time_t seconds = static_cast<time_t>(m_micro_since_epoch/k_micro_seconds_persecond);
    struct tm tm_time;
    gmtime_r(&seconds,&tm_time);
    if(show_micro){
        int microseconds = static_cast<int>(m_micro_since_epoch%k_micro_seconds_persecond);
        snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d.%06d",
             tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
             tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec,
             microseconds);
    }
    else{
        snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d",
             tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
             tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
    }
    return buf;
}
TimeStamp TimeStamp::now(){
    struct timeval tv;
    gettimeofday(&tv,NULL);
    int64_t seconds = tv.tv_sec;
    return TimeStamp(seconds*k_micro_seconds_persecond);
}