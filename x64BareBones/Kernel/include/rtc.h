#ifndef RTC_H
#define RTC_H
#include <stdint.h>
typedef struct {
    uint8_t sec;
    uint8_t min;
    uint8_t hour;
    uint8_t day;
    uint8_t month;
    uint8_t year;
} rtc_time_t;

#endif
