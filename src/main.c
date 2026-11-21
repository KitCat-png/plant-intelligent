#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define OLED_ADDR   (0x3C << 1)
#define DS3231_ADDR (0x68 << 1)

// Mantém a 0 para preservar a hora guardada no DS3231
#define ACERTAR_HORA 0
#define DIA_INICIAL  21
#define MES_INICIAL  11
#define ANO_INICIAL  26

// Limiares para as carinhas (podes ajustar facilmente para a tua planta!)
#define LUX_NOITE    20
#define TEMP_CALOR   27
#define TEMP_FRIO    17

typedef struct {
    uint8_t sec, min, hour;
    uint8_t day, month, year;
} rtc_time_t;

typedef enum {
    MOOD_FELIZ = 0,
    MOOD_CALOR,
    MOOD_FRIO,
    MOOD_NOITE
} mood_t;

// --- 1. DRIVER I2C (TWI) ---
void i2c_init(void) {
    TWSR = 0x00;
    TWBR = 72; // 100 kHz com cristal de 16 MHz
    TWCR = (1 << TWEN);
}

void i2c_start(void) {
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
}

void i2c_stop(void) {
    TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
    while (TWCR & (1 << TWSTO));
}

uint8_t i2c_write(uint8_t data) {
    TWDR = data;
    TWCR = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
    uint8_t status = TWSR & 0xF8;
    return (status == 0x18 || status == 0x28 || status == 0x40);
}

uint8_t i2c_read_ack(void) {
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
    while (!(TWCR & (1 << TWINT)));
    return TWDR;
}

uint8_t i2c_read_nack(void) {
    TWCR = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
    return TWDR;
}

uint8_t i2c_probe(uint8_t addr7) {
    i2c_start();
    uint8_t ok = i2c_write(addr7 << 1);
    i2c_stop();
    return ok;
}

// --- 2. DRIVER OLED SSD1306 ---
void oled_cmd(uint8_t cmd) {
    i2c_start();
    i2c_write(OLED_ADDR);
    i2c_write(0x00);
    i2c_write(cmd);
    i2c_stop();
}

void oled_init(void) {
    _delay_ms(100);
    oled_cmd(0xAE);
    oled_cmd(0xD5); oled_cmd(0x80);
    oled_cmd(0xA8); oled_cmd(0x3F);
    oled_cmd(0xD3); oled_cmd(0x00);
    oled_cmd(0x40);
    oled_cmd(0x8D); oled_cmd(0x14);
    oled_cmd(0x20); oled_cmd(0x00);
    oled_cmd(0xA1);
    oled_cmd(0xC8);
    oled_cmd(0xDA); oled_cmd(0x12);
    oled_cmd(0x81); oled_cmd(0xCF);
    oled_cmd(0xD9); oled_cmd(0xF1);
    oled_cmd(0xDB); oled_cmd(0x40);
    oled_cmd(0xA4);
    oled_cmd(0xA6);
    oled_cmd(0xAF);
}

void oled_set_cursor(uint8_t col, uint8_t page) {
    oled_cmd(0x21); oled_cmd(col);  oled_cmd(127);
    oled_cmd(0x22); oled_cmd(page); oled_cmd(7);
}

void oled_clear(void) {
    oled_set_cursor(0, 0);
    i2c_start();
    i2c_write(OLED_ADDR);
    i2c_write(0x40);
    for (uint16_t i = 0; i < 1024; i++) i2c_write(0x00);
    i2c_stop();
}

void oled_draw_divider(uint8_t page) {
    oled_set_cursor(0, page);
    i2c_start();
    i2c_write(OLED_ADDR);
    i2c_write(0x40);
    for (uint8_t i = 0; i < 128; i++) i2c_write(0x08);
    i2c_stop();
}

void oled_draw_char(char c) {
    static const uint8_t digits[10][5] = {
        {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 0
        {0x00, 0x42, 0x7F, 0x40, 0x00}, // 1
        {0x42, 0x61, 0x51, 0x49, 0x46}, // 2
        {0x22, 0x41, 0x49, 0x49, 0x36}, // 3
        {0x18, 0x14, 0x12, 0x7F, 0x10}, // 4
        {0x27, 0x45, 0x45, 0x45, 0x39}, // 5
        {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 6
        {0x01, 0x71, 0x09, 0x05, 0x03}, // 7
        {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
        {0x06, 0x49, 0x49, 0x29, 0x1E}  // 9
    };
    static const uint8_t g_A[5]   = {0x7E, 0x11, 0x11, 0x11, 0x7E};
    static const uint8_t g_B[5]   = {0x7F, 0x49, 0x49, 0x49, 0x36};
    static const uint8_t g_C[5]   = {0x3E, 0x41, 0x41, 0x41, 0x22};
    static const uint8_t g_D[5]   = {0x7F, 0x41, 0x41, 0x22, 0x1C};
    static const uint8_t g_E[5]   = {0x7F, 0x49, 0x49, 0x49, 0x41};
    static const uint8_t g_F[5]   = {0x7F, 0x09, 0x09, 0x09, 0x01};
    static const uint8_t g_H[5]   = {0x7F, 0x08, 0x08, 0x08, 0x7F};
    static const uint8_t g_I[5]   = {0x00, 0x41, 0x7F, 0x41, 0x00};
    static const uint8_t g_L[5]   = {0x7F, 0x40, 0x40, 0x40, 0x40};
    static const uint8_t g_M[5]   = {0x7F, 0x02, 0x0C, 0x02, 0x7F};
    static const uint8_t g_N[5]   = {0x7F, 0x04, 0x08, 0x10, 0x7F};
    static const uint8_t g_O[5]   = {0x3E, 0x41, 0x41, 0x41, 0x3E};
    static const uint8_t g_P[5]   = {0x7F, 0x09, 0x09, 0x09, 0x06};
    static const uint8_t g_R[5]   = {0x7F, 0x09, 0x19, 0x29, 0x46};
    static const uint8_t g_S[5]   = {0x46, 0x49, 0x49, 0x49, 0x31};
    static const uint8_t g_T[5]   = {0x01, 0x01, 0x7F, 0x01, 0x01};
    static const uint8_t g_Z[5]   = {0x61, 0x51, 0x49, 0x45, 0x43};
    static const uint8_t g_h[5]   = {0x7F, 0x08, 0x04, 0x04, 0x78};
    static const uint8_t g_a[5]   = {0x20, 0x54, 0x54, 0x54, 0x78};
    static const uint8_t g_l[5]   = {0x00, 0x41, 0x7F, 0x40, 0x00};
    static const uint8_t g_x[5]   = {0x44, 0x28, 0x10, 0x28, 0x44};
    static const uint8_t g_dot[5] = {0x00, 0x60, 0x60, 0x00, 0x00};
    static const uint8_t g_col[5] = {0x00, 0x36, 0x36, 0x00, 0x00};
    static const uint8_t g_min[5] = {0x08, 0x08, 0x08, 0x08, 0x08};
    static const uint8_t g_sl[5]  = {0x20, 0x10, 0x08, 0x04, 0x02};
    static const uint8_t g_pct[5] = {0x23, 0x13, 0x08, 0x64, 0x62};
    static const uint8_t g_sp[5]  = {0x00, 0x00, 0x00, 0x00, 0x00};

    const uint8_t *glyph = g_sp;
    if (c >= '0' && c <= '9') {
        glyph = digits[c - '0'];
    } else {
        switch (c) {
            case 'A': glyph = g_A; break;
            case 'B': glyph = g_B; break;
            case 'C': glyph = g_C; break;
            case 'D': glyph = g_D; break;
            case 'E': glyph = g_E; break;
            case 'F': glyph = g_F; break;
            case 'H': glyph = g_H; break;
            case 'I': glyph = g_I; break;
            case 'L': glyph = g_L; break;
            case 'M': glyph = g_M; break;
            case 'N': glyph = g_N; break;
            case 'O': glyph = g_O; break;
            case 'P': glyph = g_P; break;
            case 'R': glyph = g_R; break;
            case 'S': glyph = g_S; break;
            case 'T': glyph = g_T; break;
            case 'Z': glyph = g_Z; break;
            case 'h': glyph = g_h; break;
            case 'a': glyph = g_a; break;
            case 'l': glyph = g_l; break;
            case 'x': glyph = g_x; break;
            case '.': glyph = g_dot; break;
            case ':': glyph = g_col; break;
            case '-': glyph = g_min; break;
            case '/': glyph = g_sl; break;
            case '%': glyph = g_pct; break;
        }
    }

    i2c_start();
    i2c_write(OLED_ADDR);
    i2c_write(0x40);
    for (uint8_t i = 0; i < 5; i++) i2c_write(glyph[i]);
    i2c_write(0x00);
    i2c_stop();
}

void oled_print(const char *str) {
    while (*str) oled_draw_char(*str++);
}

// Garante largura fixa para nunca invadir a zona da carinha nem deixar lixo
void oled_print_padded(const char *str, uint8_t width) {
    uint8_t i = 0;
    while (i < width && str[i]) {
        oled_draw_char(str[i++]);
    }
    while (i < width) {
        oled_draw_char(' ');
        i++;
    }
}

// --- 3. MOTOR DE CARINHAS ANIMADAS (32x32 PÍXEIS EM C PURO) ---
static void fb_px(uint8_t buf[4][32], int8_t x, int8_t y) {
    if (x >= 0 && x < 32 && y >= 0 && y < 32) {
        buf[y >> 3][x] |= (uint8_t)(1 << (y & 7));
    }
}

static void fb_rect(uint8_t buf[4][32], int8_t x0, int8_t y0, int8_t w, int8_t h) {
    for (int8_t y = y0; y < y0 + h; y++) {
        for (int8_t x = x0; x < x0 + w; x++) {
            fb_px(buf, x, y);
        }
    }
}

static void fb_circle_outline(uint8_t buf[4][32], int8_t sx) {
    for (int8_t y = 0; y < 32; y++) {
        int16_t dy = 2 * y - 31;
        for (int8_t x = 0; x < 32; x++) {
            int16_t dx = 2 * (x - sx) - 31;
            int16_t d2 = dx * dx + dy * dy;
            if (d2 >= 760 && d2 <= 870) {
                fb_px(buf, x, y);
            }
        }
    }
}

void oled_draw_face(uint8_t col_start, uint8_t page_start, mood_t mood, uint8_t frame) {
    uint8_t buf[4][32];
    memset(buf, 0, sizeof(buf));

    int8_t sx = (mood == MOOD_FRIO) ? ((frame & 1) ? 1 : -1) : 0;
    fb_circle_outline(buf, sx);

    if (mood == MOOD_FELIZ) {
        if (frame == 7) {
            // Piscar de olhos feliz (^ ^)
            fb_px(buf, 9, 11);  fb_px(buf, 10, 10); fb_px(buf, 11, 11);
            fb_px(buf, 20, 11); fb_px(buf, 21, 10); fb_px(buf, 22, 11);
        } else {
            // Olhos abertos redondinhos
            fb_rect(buf, 9, 10, 3, 3);
            fb_rect(buf, 20, 10, 3, 3);
        }
        // Sorriso aberto
        fb_px(buf, 10, 19); fb_px(buf, 11, 20); fb_px(buf, 12, 21);
        fb_rect(buf, 13, 22, 6, 1);
        fb_px(buf, 19, 21); fb_px(buf, 20, 20); fb_px(buf, 21, 19);

    } else if (mood == MOOD_CALOR) {
        // Sobrancelhas preocupadas + olhos cansados
        fb_px(buf, 8, 9);   fb_px(buf, 9, 9);   fb_px(buf, 10, 8);
        fb_px(buf, 21, 8);  fb_px(buf, 22, 9);  fb_px(buf, 23, 9);
        fb_rect(buf, 9, 11, 3, 2);
        fb_rect(buf, 20, 11, 3, 2);
        // Boca triste
        fb_px(buf, 11, 22); fb_px(buf, 12, 21);
        fb_rect(buf, 13, 20, 6, 1);
        fb_px(buf, 19, 21); fb_px(buf, 20, 22);
        // Gota de suor animada a descer
        int8_t yd = 6 + (int8_t)((frame % 3) * 2);
        fb_px(buf, 24, yd);
        fb_rect(buf, 23, yd + 1, 3, 1);
        fb_px(buf, 24, yd + 2);

    } else if (mood == MOOD_FRIO) {
        // Olhos arregalados a tremer com a cabeça
        fb_rect(buf, 9 + sx, 10, 3, 3);
        fb_rect(buf, 20 + sx, 10, 3, 3);
        // Boca em ziguezague (bater o dente)
        for (int8_t x = 10; x <= 21; x++) {
            int8_t y = (x & 2) ? 20 : 21;
            fb_px(buf, x + sx, y);
        }

    } else { // MOOD_NOITE
        // Olhos fechados a dormir (u u)
        fb_px(buf, 8, 12);  fb_rect(buf, 9, 13, 3, 1);  fb_px(buf, 12, 12);
        fb_px(buf, 19, 12); fb_rect(buf, 20, 13, 3, 1); fb_px(buf, 23, 12);
        // Boca a respirar devagar
        if ((frame >> 1) & 1) {
            fb_rect(buf, 14, 19, 4, 1);
            fb_px(buf, 14, 20); fb_px(buf, 17, 20);
            fb_rect(buf, 14, 21, 4, 1);
        } else {
            fb_rect(buf, 15, 20, 2, 2);
        }
        // Pequeno "z" a flutuar no canto superior direito
        int8_t zy = 6 + (int8_t)((frame >> 1) & 1);
        fb_rect(buf, 21, zy, 4, 1);
        fb_px(buf, 23, zy + 1);
        fb_px(buf, 22, zy + 2);
        fb_rect(buf, 21, zy + 3, 4, 1);
    }

    // Envia as 4 páginas (32x32) para o OLED
    for (uint8_t p = 0; p < 4; p++) {
        oled_set_cursor(col_start, page_start + p);
        i2c_start();
        i2c_write(OLED_ADDR);
        i2c_write(0x40);
        for (uint8_t x = 0; x < 32; x++) {
            i2c_write(buf[p][x]);
        }
        i2c_stop();
    }
}

// --- 4. DRIVER DS3231 (RELÓGIO RTC) ---
static uint8_t dec_to_bcd(uint8_t val) {
    return ((val / 10) << 4) | (val % 10);
}

static uint8_t bcd_to_dec(uint8_t val) {
    return ((val >> 4) * 10) + (val & 0x0F);
}

void ds3231_set_time(uint8_t sec, uint8_t min, uint8_t hour, uint8_t day, uint8_t month, uint8_t year) {
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

// --- 5. DRIVER BH1750 (LUMINOSIDADE) ---
static uint8_t bh_addr = 0;

void bh1750_cmd(uint8_t cmd) {
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

// --- 6. DRIVER BME280 (AMBIENTAL) ---
static uint8_t  bme_addr = 0;
static uint16_t dig_T1, dig_P1;
static int16_t  dig_T2, dig_T3;
static int16_t  dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;
static uint8_t  dig_H1, dig_H3;
static int16_t  dig_H2, dig_H4, dig_H5;
static int8_t   dig_H6;
static int32_t  t_fine;

void bme_write_reg(uint8_t reg, uint8_t val) {
    i2c_start();
    i2c_write(bme_addr);
    i2c_write(reg);
    i2c_write(val);
    i2c_stop();
}

void bme_read_regs(uint8_t reg, uint8_t *buf, uint8_t len) {
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

uint8_t bme280_init(void) {
    if (i2c_probe(0x76))      bme_addr = (0x76 << 1);
    else if (i2c_probe(0x77)) bme_addr = (0x77 << 1);
    else return 0;

    uint8_t chip_id = 0;
    bme_read_regs(0xD0, &chip_id, 1);

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
    return chip_id;
}

int32_t bme_comp_T(int32_t adc_T) {
    int32_t var1 = ((((adc_T >> 3) - ((int32_t)dig_T1 << 1))) * ((int32_t)dig_T2)) >> 11;
    int32_t var2 = (((((adc_T >> 4) - ((int32_t)dig_T1)) * ((adc_T >> 4) - ((int32_t)dig_T1))) >> 12) * ((int32_t)dig_T3)) >> 14;
    t_fine = var1 + var2;
    return (t_fine * 5 + 128) >> 8;
}

uint32_t bme_comp_P(int32_t adc_P) {
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

uint32_t bme_comp_H(int32_t adc_H) {
    int32_t v = (t_fine - ((int32_t)76800));
    v = (((((adc_H << 14) - (((int32_t)dig_H4) << 20) - (((int32_t)dig_H5) * v)) + ((int32_t)16384)) >> 15) *
        (((((((v * ((int32_t)dig_H6)) >> 10) * (((v * ((int32_t)dig_H3)) >> 11) + ((int32_t)32768))) >> 10) +
        ((int32_t)2097152)) * ((int32_t)dig_H2) + 8192) >> 14));
    v = (v - (((((v >> 15) * (v >> 15)) >> 7) * ((int32_t)dig_H1)) >> 4));
    if (v < 0) v = 0;
    if (v > 419430400) v = 419430400;
    return (uint32_t)(v >> 12);
}

// --- PROGRAMA PRINCIPAL ---
int main(void) {
    char buf[22];
    rtc_time_t agora;
    uint16_t lux = 0;
    uint8_t frame = 0;

    i2c_init();
    oled_init();
    oled_clear();
    oled_draw_divider(1);

    uint8_t rtc_ok = i2c_probe(0x68);
    uint8_t bme_id = bme280_init();
    uint8_t bh_ok  = bh1750_init();

#if ACERTAR_HORA
    if (rtc_ok) {
        uint8_t h = (__TIME__[0] - '0') * 10 + (__TIME__[1] - '0');
        uint8_t m = (__TIME__[3] - '0') * 10 + (__TIME__[4] - '0');
        uint8_t s = (__TIME__[6] - '0') * 10 + (__TIME__[7] - '0');
        ds3231_set_time(s, m, h, DIA_INICIAL, MES_INICIAL, ANO_INICIAL);
    }
#endif

    while (1) {
        // --- TOPO (Página 0): Relógio DS3231 ---
        oled_set_cursor(13, 0);
        if (rtc_ok && ds3231_read_time(&agora)) {
            snprintf(buf, sizeof(buf), "%02u/%02u/%02u %02u:%02u:%02u",
                     agora.day, agora.month, agora.year,
                     agora.hour, agora.min, agora.sec);
            oled_print(buf);
        } else {
            oled_print("DS3231 ERRO      ");
        }

        // --- ESQUERDA (Páginas 2, 3, 5, 6): Leituras dos Sensores ---
        int16_t t_int = 22;
        uint8_t bme_valid = 0;

        if (bme_id == 0x60 || bme_id == 0x58) {
            uint8_t raw[8];
            bme_read_regs(0xF7, raw, 8);

            int32_t adc_P = ((int32_t)raw[0] << 12) | ((int32_t)raw[1] << 4) | (raw[2] >> 4);
            int32_t adc_T = ((int32_t)raw[3] << 12) | ((int32_t)raw[4] << 4) | (raw[5] >> 4);
            int32_t adc_H = ((int32_t)raw[6] << 8)  | raw[7];

            int32_t  temp_x100 = bme_comp_T(adc_T);
            uint32_t press_pa  = bme_comp_P(adc_P);
            uint32_t hum_q10   = bme_comp_H(adc_H);

            t_int = (int16_t)(temp_x100 / 100);
            int16_t  t_dec = ((temp_x100 < 0 ? -temp_x100 : temp_x100) % 100) / 10;
            uint16_t p_hpa = (uint16_t)(press_pa / 100UL);
            uint16_t h_pct = (uint16_t)(hum_q10 >> 10);
            bme_valid = 1;

            oled_set_cursor(0, 2);
            snprintf(buf, sizeof(buf), "T:%d.%d C", t_int, t_dec);
            oled_print_padded(buf, 13);

            oled_set_cursor(0, 3);
            if (bme_id == 0x60) {
                snprintf(buf, sizeof(buf), "H:%u %%", h_pct);
            } else {
                snprintf(buf, sizeof(buf), "H:--");
            }
            oled_print_padded(buf, 13);

            oled_set_cursor(0, 6);
            snprintf(buf, sizeof(buf), "P:%u hPa", p_hpa);
            oled_print_padded(buf, 13);
        } else {
            oled_set_cursor(0, 2);
            oled_print_padded("BME ERRO", 13);
        }

        uint8_t bh_valid = 0;
        oled_set_cursor(0, 5);
        if (bh_ok && bh1750_read_lux(&lux)) {
            snprintf(buf, sizeof(buf), "L:%u lx", lux);
            oled_print_padded(buf, 13);
            bh_valid = 1;
        } else {
            oled_print_padded("BH1750 ERRO", 13);
        }

        // --- DIREITA (Páginas 2..5 + legenda na 6): Carinha Animada ---
        mood_t mood = MOOD_FELIZ;

        if (bh_valid && lux < LUX_NOITE) {
            mood = MOOD_NOITE;
            
        } else if (bme_valid && t_int >= TEMP_CALOR) {
            mood = MOOD_CALOR;

        } else if (bme_valid && t_int <= TEMP_FRIO) {
            mood = MOOD_FRIO;
        }

        oled_draw_face(88, 3, mood, frame);

        frame = (frame + 1) & 0x07;
        _delay_ms(400);
    }
    return 0;
}