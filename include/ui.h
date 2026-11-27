#ifndef UI_H
#define UI_H

#include <stdint.h>

typedef enum {
    MOOD_FELIZ = 0,
    MOOD_SEDE,
    MOOD_ENCHARCADO,
    MOOD_CALOR,
    MOOD_FRIO,
    MOOD_NOITE
} mood_t;

void ui_init_icons(void);
void ui_draw_face48(uint8_t col_start, uint8_t page_start, mood_t mood, uint8_t frame);

#endif