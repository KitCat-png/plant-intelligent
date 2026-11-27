#ifndef BME280_H
#define BME280_H

#include <stdint.h>

typedef struct {
    int16_t  temp_int;
    int16_t  temp_dec;
    uint16_t press_hpa;
    uint16_t hum_pct;
    uint8_t  has_hum;
} bme280_data_t;

uint8_t bme280_init(void);
uint8_t bme280_read(bme280_data_t *out);

#endif