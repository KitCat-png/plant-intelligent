#include "oled.h"
#include "config.h"
#include "i2c.h"
#include <util/delay.h>

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

void oled_write_page(uint8_t col, uint8_t page, const uint8_t *data, uint8_t len) {
    oled_set_cursor(col, page);
    i2c_start();
    i2c_write(OLED_ADDR);
    i2c_write(0x40);
    for (uint8_t i = 0; i < len; i++) i2c_write(data[i]);
    i2c_stop();
}

void oled_draw_icon8(uint8_t col, uint8_t page, const uint8_t icon[8]) {
    oled_write_page(col, page, icon, 8);
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
    static const uint8_t g_G[5]   = {0x3E, 0x41, 0x49, 0x49, 0x7A};
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
    static const uint8_t g_U[5]   = {0x3F, 0x40, 0x40, 0x40, 0x3F};
    static const uint8_t g_W[5]   = {0x3F, 0x40, 0x38, 0x40, 0x3F};
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
    static const uint8_t g_deg[5] = {0x06, 0x09, 0x09, 0x06, 0x00};
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
            case 'G': glyph = g_G; break;
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
            case 'U': glyph = g_U; break;
            case 'W': glyph = g_W; break;
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
            case '*': glyph = g_deg; break;
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

void oled_print_padded(const char *str, uint8_t width) {
    uint8_t i = 0;
    while (i < width && str[i]) oled_draw_char(str[i++]);
    while (i < width) {
        oled_draw_char(' ');
        i++;
    }
}