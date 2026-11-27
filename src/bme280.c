#include "bme280.h"
#include "i2c.h"

static uint8_t  bme_addr = 0;
static uint8_t  bme_id   = 0;
static uint16_t dig_T1, dig_P1;
static int16_t  dig_T2, dig_T3;
static int16_t  dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;
static uint8_t  dig_H1, dig_H3;
static int16_t  dig_H2, dig_H4, dig_H5;
static int8_t   dig_H6;
static int32_t  t_fine;

static void bme_write_reg(uint8_t reg, uint8_t val) {
    i2c_start();
    i2c_write(bme_addr);
    i2c_write(reg);
    i2c_write(val);
    i2c_stop();
}

static void bme_read_regs(uint8_t reg, uint8_t *buf, uint8_t len) {
    i2c_start();
    i2c_write(bme_addr);
    i2c_write(reg);
    i2c_start();
    i2c_write(bme_addr | 1);
    for (uint8_t i = 0; i < len; i++) {
        buf[i] = (i < len - 1) ? i2c_read_ack() : i2c_read_nack();
    }
    i2c_stop();
}

static int32_t bme_comp_T(int32_t adc_T) {
    int32_t var1 = ((((adc_T >> 3) - ((int32_t)dig_T1 << 1))) * ((int32_t)dig_T2)) >> 11;
    int32_t var2 = (((((adc_T >> 4) - ((int32_t)dig_T1)) * ((adc_T >> 4) - ((int32_t)dig_T1))) >> 12) * ((int32_t)dig_T3)) >> 14;
    t_fine = var1 + var2;
    return (t_fine * 5 + 128) >> 8;
}

static uint32_t bme_comp_P(int32_t adc_P) {
    int32_t var1 = (((int32_t)t_fine) >> 1) - (int32_t)64000;
    int32_t var2 = (((var1 >> 2) * (var1 >> 2)) >> 11) * ((int32_t)dig_P6);
    var2 = var2 + ((var1 * ((int32_t)dig_P5)) << 1);
    var2 = (var2 >> 2) + (((int32_t)dig_P4) << 16);
    var1 = ((((int32_t)dig_P3 * (((var1 >> 2) * (var1 >> 2)) >> 13)) >> 3) + ((((int32_t)dig_P2) * var1) >> 1)) >> 18;
    var1 = ((((32768L + var1)) * ((int32_t)dig_P1)) >> 15);
    if (var1 == 0) return 0;

    uint32_t p = (((uint32_t)(((int32_t)1048576) - adc_P) - (var2 >> 12))) * 3125UL;
    if (p < 0x80000000UL) p = (p << 1) / ((uint32_t)var1);
    else                  p = (p / (uint32_t)var1) * 2;

    var1 = (((int32_t)dig_P9) * ((int32_t)(((p >> 3) * (p >> 3)) >> 13))) >> 12;
    var2 = (((int32_t)(p >> 2)) * ((int32_t)dig_P8)) >> 13;
    return (uint32_t)((int32_t)p + ((var1 + var2 + dig_P7) >> 4));
}

static uint32_t bme_comp_H(int32_t adc_H) {
    int32_t v = (t_fine - ((int32_t)76800));
    v = (((((adc_H << 14) - (((int32_t)dig_H4) << 20) - (((int32_t)dig_H5) * v)) + ((int32_t)16384)) >> 15) *
        (((((((v * ((int32_t)dig_H6)) >> 10) * (((v * ((int32_t)dig_H3)) >> 11) + ((int32_t)32768))) >> 10) +
        ((int32_t)2097152)) * ((int32_t)dig_H2) + 8192) >> 14));
    v = (v - (((((v >> 15) * (v >> 15)) >> 7) * ((int32_t)dig_H1)) >> 4));
    if (v < 0) v = 0;
    if (v > 419430400) v = 419430400;
    return (uint32_t)(v >> 12);
}

uint8_t bme280_init(void) {
    if (i2c_probe(0x76))      bme_addr = (0x76 << 1);
    else if (i2c_probe(0x77)) bme_addr = (0x77 << 1);
    else return 0;

    bme_read_regs(0xD0, &bme_id, 1);

    uint8_t b[24];
    bme_read_regs(0x88, b, 24);
    dig_T1 = (uint16_t)b[0]  | ((uint16_t)b[1]  << 8);
    dig_T2 = (int16_t)((uint16_t)b[2]  | ((uint16_t)b[3]  << 8));
    dig_T3 = (int16_t)((uint16_t)b[4]  | ((uint16_t)b[5]  << 8));
    dig_P1 = (uint16_t)b[6]  | ((uint16_t)b[7]  << 8);
    dig_P2 = (int16_t)((uint16_t)b[8]  | ((uint16_t)b[9]  << 8));
    dig_P3 = (int16_t)((uint16_t)b[10] | ((uint16_t)b[11] << 8));
    dig_P4 = (int16_t)((uint16_t)b[12] | ((uint16_t)b[13] << 8));
    dig_P5 = (int16_t)((uint16_t)b[14] | ((uint16_t)b[15] << 8));
    dig_P6 = (int16_t)((uint16_t)b[16] | ((uint16_t)b[17] << 8));
    dig_P7 = (int16_t)((uint16_t)b[18] | ((uint16_t)b[19] << 8));
    dig_P8 = (int16_t)((uint16_t)b[20] | ((uint16_t)b[21] << 8));
    dig_P9 = (int16_t)((uint16_t)b[22] | ((uint16_t)b[23] << 8));

    bme_read_regs(0xA1, &dig_H1, 1);
    uint8_t h[7];
    bme_read_regs(0xE1, h, 7);
    dig_H2 = (int16_t)((uint16_t)h[0] | ((uint16_t)h[1] << 8));
    dig_H3 = h[2];
    dig_H4 = (int16_t)(((int16_t)(int8_t)h[3] << 4) | (h[4] & 0x0F));
    dig_H5 = (int16_t)(((int16_t)(int8_t)h[5] << 4) | (h[4] >> 4));
    dig_H6 = (int8_t)h[6];

    bme_write_reg(0xF2, 0x01);
    bme_write_reg(0xF5, 0xA0);
    bme_write_reg(0xF4, 0x27);
    return bme_id;
}

uint8_t bme280_read(bme280_data_t *out) {
    if (bme_id != 0x60 && bme_id != 0x58) return 0;

    uint8_t raw[8];
    bme_read_regs(0xF7, raw, 8);

    int32_t adc_P = ((int32_t)raw[0] << 12) | ((int32_t)raw[1] << 4) | (raw[2] >> 4);
    int32_t adc_T = ((int32_t)raw[3] << 12) | ((int32_t)raw[4] << 4) | (raw[5] >> 4);
    int32_t adc_H = ((int32_t)raw[6] << 8)  | raw[7];

    int32_t  temp_x100 = bme_comp_T(adc_T);
    uint32_t press_pa  = bme_comp_P(adc_P);
    uint32_t hum_q10   = bme_comp_H(adc_H);

    out->temp_int  = (int16_t)(temp_x100 / 100);
    out->temp_dec  = (int16_t)(((temp_x100 < 0 ? -temp_x100 : temp_x100) % 100) / 10);
    out->press_hpa = (uint16_t)(press_pa / 100UL);
    out->hum_pct   = (uint16_t)(hum_q10 >> 10);
    out->has_hum   = (bme_id == 0x60);
    return 1;
}