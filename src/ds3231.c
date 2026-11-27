#include "ds3231.h"
#include "config.h"
#include "i2c.h"

static uint8_t dec_to_bcd(uint8_t val) {
    return (uint8_t)(((val / 10) << 4) | (val % 10));
}

static uint8_t bcd_to_dec(uint8_t val) {
    return (uint8_t)(((val >> 4) * 10) + (val & 0x0F));
}

uint8_t ds3231_init(void) {
    return i2c_probe(DS3231_ADDR >> 1);
}

void ds3231_set_time(uint8_t sec, uint8_t min, uint8_t hour,
                     uint8_t day, uint8_t month, uint8_t year) {
    i2c_start();
    i2c_write(DS3231_ADDR);
    i2c_write(0x00);
    i2c_write(dec_to_bcd(sec));
    i2c_write(dec_to_bcd(min));
    i2c_write(dec_to_bcd(hour));
    i2c_write(1);
    i2c_write(dec_to_bcd(day));
    i2c_write(dec_to_bcd(month));
    i2c_write(dec_to_bcd(year));
    i2c_stop();
}

uint8_t ds3231_read_time(rtc_time_t *t) {
    uint8_t raw[7];
    i2c_start();
    if (!i2c_write(DS3231_ADDR)) {
        i2c_stop();
        return 0;
    }
    i2c_write(0x00);
    i2c_start();
    i2c_write(DS3231_ADDR | 1);
    for (uint8_t i = 0; i < 7; i++) {
        raw[i] = (i < 6) ? i2c_read_ack() : i2c_read_nack();
    }
    i2c_stop();

    t->sec   = bcd_to_dec(raw[0] & 0x7F);
    t->min   = bcd_to_dec(raw[1] & 0x7F);
    t->hour  = bcd_to_dec(raw[2] & 0x3F);
    t->day   = bcd_to_dec(raw[4] & 0x3F);
    t->month = bcd_to_dec(raw[5] & 0x1F);
    t->year  = bcd_to_dec(raw[6]);
    return 1;
}