#ifndef OLED_H
#define OLED_H

#include <stdint.h>

void oled_cmd(uint8_t cmd);
void oled_init(void);
void oled_set_cursor(uint8_t col, uint8_t page);
void oled_clear(void);
void oled_draw_divider(uint8_t page);
void oled_write_page(uint8_t col, uint8_t page, const uint8_t *data, uint8_t len);
void oled_draw_icon8(uint8_t col, uint8_t page, const uint8_t icon[8]);
void oled_draw_char(char c);
void oled_print(const char *str);
void oled_print_padded(const char *str, uint8_t width);

#endif