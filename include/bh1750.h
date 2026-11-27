#ifndef BH1750_H
#define BH1750_H

#include <stdint.h>

uint8_t bh1750_init(void);
uint8_t bh1750_read_lux(uint16_t *lux);

#endif