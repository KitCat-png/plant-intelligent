#include "bh1750.h"
#include "config.h"
#include "i2c.h"
#include <util/delay.h>

static uint8_t bh_addr = 0;

static void bh1750_cmd(uint8_t cmd) {
    i2c_start();
    i2c_write(bh_addr);
    i2c_write(cmd);
    i2c_stop();
}

uint8_t bh1750_init(void) {
    if (i2c_probe(0x23))      bh_addr = (0x23 << 1);
    else if (i2c_probe(0x5C)) bh_addr = (0x5C << 1);
    else return 0;

    bh1750_cmd(0x01); // Power ON
    _delay_ms(10);
    bh1750_cmd(0x10); // Modo contínuo alta resolução (1 lx)
    _delay_ms(180);
    return 1;
}

uint8_t bh1750_read_lux(uint16_t *lux) {
    if (!bh_addr) return 0;
    i2c_start();
    if (!i2c_write(bh_addr | 1)) {
        i2c_stop();
        return 0;
    }
    uint8_t msb = i2c_read_ack();
    uint8_t lsb = i2c_read_nack();
    i2c_stop();

    uint16_t raw = ((uint16_t)msb << 8) | lsb;
    *lux = (uint16_t)(((uint32_t)raw * 10UL) / 12UL);
    return 1;
}