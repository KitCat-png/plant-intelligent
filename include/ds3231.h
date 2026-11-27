#ifndef DS3231_H
#define DS3231_H

#include <stdint.h>

typedef struct {
    uint8_t sec, min, hour;
    uint8_t day, month, year;
} rtc_time_t;

uint8_t ds3231_init(void);
void    ds3231_set_time(uint8_t sec, uint8_t min, uint8_t hour,
                        uint8_t day, uint8_t month, uint8_t year);
uint8_t ds3231_read_time(rtc_time_t *t);

#endif