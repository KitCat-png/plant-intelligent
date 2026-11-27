#ifndef ADC_H
#define ADC_H

#include <stdint.h>

void     adc_init(void);
uint16_t adc_read_avg(uint8_t channel);
uint8_t  soil_raw_to_pct(uint16_t raw);

#endif