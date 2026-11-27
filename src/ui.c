#include "ui.h"
#include "oled.h"
#include <string.h>

static const uint8_t icon_therm[8] = {0x00, 0x70, 0xFE, 0xF9, 0xFE, 0x70, 0x0A, 0x00};
static const uint8_t icon_cloud[8] = {0x30, 0x7C, 0x7E, 0x7E, 0x7C, 0x78, 0x78, 0x30};
static const uint8_t icon_drop[8]  = {0x00, 0x78, 0xCC, 0xFF, 0xFC, 0x78, 0x00, 0x00};

void ui_init_icons(void) {
    // Página 5 fica vazia para evitar a zona de píxeis mortos do OLED
    oled_draw_icon8(2, 2, icon_therm);
    oled_draw_icon8(2, 4, icon_cloud);
    oled_draw_icon8(2, 6, icon_drop);
}

// --- Motor Tamagotchi (24x24 escalado 2x -> 48x48 píxeis) ---
static void tama_px(uint8_t buf[6][48], int8_t x, int8_t y) {
    if (x >= 0 && x < 24 && y >= 0 && y < 24) {
        uint8_t page = (uint8_t)(y >> 2);
        uint8_t mask = (uint8_t)(0x03 << ((y & 3) << 1));
        uint8_t px   = (uint8_t)(x << 1);
        buf[page][px]     |= mask;
        buf[page][px + 1] |= mask;
    }
}

static void tama_clr(uint8_t buf[6][48], int8_t x, int8_t y) {
    if (x >= 0 && x < 24 && y >= 0 && y < 24) {
        uint8_t page = (uint8_t)(y >> 2);
        uint8_t mask = (uint8_t)(0x03 << ((y & 3) << 1));
        uint8_t px   = (uint8_t)(x << 1);
        buf[page][px]     &= (uint8_t)~mask;
        buf[page][px + 1] &= (uint8_t)~mask;
    }
}

static void tama_hline(uint8_t buf[6][48], int8_t x0, int8_t x1, int8_t y) {
    for (int8_t x = x0; x <= x1; x++) tama_px(buf, x, y);
}

static void tama_rect(uint8_t buf[6][48], int8_t x0, int8_t y0, int8_t w, int8_t h) {
    for (int8_t y = y0; y < y0 + h; y++) {
        for (int8_t x = x0; x < x0 + w; x++) tama_px(buf, x, y);
    }
}

static void tama_body(uint8_t buf[6][48], int8_t sx, int8_t dy, uint8_t show_feet) {
    tama_hline(buf, 7 + sx, 16 + sx, 6 + dy);
    tama_hline(buf, 5 + sx, 6 + sx,  7 + dy);
    tama_hline(buf, 17 + sx, 18 + sx, 7 + dy);
    tama_px(buf, 4 + sx, 8 + dy);  tama_px(buf, 19 + sx, 8 + dy);
    tama_px(buf, 3 + sx, 9 + dy);  tama_px(buf, 20 + sx, 9 + dy);
    tama_px(buf, 3 + sx, 10 + dy); tama_px(buf, 20 + sx, 10 + dy);

    for (int8_t y = 11; y <= 16; y++) {
        tama_px(buf, 2 + sx, y + dy);
        tama_px(buf, 21 + sx, y + dy);
    }

    tama_px(buf, 3 + sx, 17 + dy); tama_px(buf, 20 + sx, 17 + dy);
    tama_px(buf, 3 + sx, 18 + dy); tama_px(buf, 20 + sx, 18 + dy);
    tama_hline(buf, 4 + sx, 5 + sx, 19 + dy);
    tama_hline(buf, 18 + sx, 19 + sx, 19 + dy);
    tama_hline(buf, 6 + sx, 17 + sx, 20 + dy);

    if (show_feet) {
        tama_hline(buf, 7 + sx, 8 + sx, 21 + dy);
        tama_hline(buf, 15 + sx, 16 + sx, 21 + dy);
        tama_hline(buf, 6 + sx, 8 + sx, 22 + dy);
        tama_hline(buf, 15 + sx, 17 + sx, 22 + dy);
    }
}

static void tama_sprout(uint8_t buf[6][48], int8_t sx, int8_t dy, uint8_t droopy) {
    if (!droopy) {
        tama_hline(buf, 8 + sx, 9 + sx, 1 + dy);
        tama_hline(buf, 14 + sx, 15 + sx, 1 + dy);
        tama_hline(buf, 7 + sx, 10 + sx, 2 + dy);
        tama_hline(buf, 13 + sx, 16 + sx, 2 + dy);
        tama_hline(buf, 8 + sx, 10 + sx, 3 + dy);
        tama_hline(buf, 13 + sx, 15 + sx, 3 + dy);
        tama_hline(buf, 10 + sx, 13 + sx, 4 + dy);
        tama_hline(buf, 11 + sx, 12 + sx, 5 + dy);
    } else {
        tama_hline(buf, 9 + sx, 14 + sx, 2 + dy);
        tama_hline(buf, 8 + sx, 9 + sx, 3 + dy);
        tama_hline(buf, 11 + sx, 12 + sx, 3 + dy);
        tama_hline(buf, 14 + sx, 15 + sx, 3 + dy);
        tama_hline(buf, 7 + sx, 8 + sx, 4 + dy);
        tama_hline(buf, 11 + sx, 12 + sx, 4 + dy);
        tama_hline(buf, 15 + sx, 16 + sx, 4 + dy);
        tama_hline(buf, 11 + sx, 12 + sx, 5 + dy);
    }
}

void ui_draw_face48(uint8_t col_start, uint8_t page_start, mood_t mood, uint8_t frame) {
    uint8_t buf[6][48];
    memset(buf, 0, sizeof(buf));

    int8_t sx = (mood == MOOD_FRIO) ? ((frame & 1) ? 1 : -1) : 0;
    int8_t dy = 0;
    uint8_t show_feet = 1;
    uint8_t droopy = (mood == MOOD_SEDE || mood == MOOD_CALOR);

    if (mood == MOOD_FELIZ) {
        dy = ((frame >> 1) & 1) ? -1 : 0;
    } else if (mood == MOOD_ENCHARCADO) {
        dy = ((frame >> 1) & 1) ? 1 : 0;
        show_feet = 0;
    } else if (mood == MOOD_NOITE) {
        dy = 1;
        show_feet = 0;
    }

    tama_body(buf, sx, dy, show_feet);
    tama_sprout(buf, sx, dy, droopy);

    if (mood == MOOD_FELIZ) {
        if (frame == 7) {
            tama_px(buf, 6, 12 + dy);  tama_hline(buf, 7, 8, 11 + dy);   tama_px(buf, 9, 12 + dy);
            tama_px(buf, 14, 12 + dy); tama_hline(buf, 15, 16, 11 + dy); tama_px(buf, 17, 12 + dy);
        } else {
            tama_rect(buf, 7, 11 + dy, 2, 3);  tama_clr(buf, 7, 11 + dy);
            tama_rect(buf, 15, 11 + dy, 2, 3); tama_clr(buf, 15, 11 + dy);
        }

        tama_hline(buf, 4, 5, 14 + dy);
        tama_hline(buf, 18, 19, 14 + dy);

        if (dy < 0) {
            tama_hline(buf, 10, 13, 15 + dy);
            tama_hline(buf, 11, 12, 16 + dy);
            tama_px(buf, 21, 2);
            tama_hline(buf, 20, 22, 3);
            tama_px(buf, 21, 4);
        } else {
            tama_px(buf, 10, 15 + dy);
            tama_hline(buf, 11, 12, 16 + dy);
            tama_px(buf, 13, 15 + dy);
        }

    } else if (mood == MOOD_SEDE) {
        tama_hline(buf, 6, 7, 10);  tama_px(buf, 8, 9);
        tama_px(buf, 15, 9);        tama_hline(buf, 16, 17, 10);

        tama_rect(buf, 7, 11, 2, 3);  tama_clr(buf, 7, 11);
        tama_rect(buf, 15, 11, 2, 3); tama_clr(buf, 15, 11);

        tama_hline(buf, 9, 14, 15);
        tama_hline(buf, 11, 12, 16);
        if (frame & 1) tama_hline(buf, 11, 12, 17);

        int8_t yd = 1 + (int8_t)(frame & 3);
        tama_px(buf, 21, yd);
        tama_px(buf, 20, yd + 1); tama_px(buf, 22, yd + 1);
        tama_hline(buf, 20, 22, yd + 2);

    } else if (mood == MOOD_ENCHARCADO) {
        tama_rect(buf, 6, 11 + dy, 3, 3);  tama_clr(buf, 7, 12 + dy);
        tama_rect(buf, 15, 11 + dy, 3, 3); tama_clr(buf, 16, 12 + dy);

        if (frame & 1) {
            tama_hline(buf, 11, 12, 15 + dy);
            tama_px(buf, 10, 16 + dy);
            tama_px(buf, 13, 16 + dy);
            tama_hline(buf, 11, 12, 17 + dy);
        } else {
            tama_rect(buf, 11, 16 + dy, 2, 2);
        }

        for (int8_t y = 20; y <= 23; y++) {
            for (int8_t x = 0; x < 24; x++) tama_clr(buf, x, y);
        }

        for (int8_t x = 1; x <= 22; x++) {
            int8_t wy = (((uint8_t)(x + frame) & 3) < 2) ? 20 : 21;
            tama_px(buf, x, wy);
        }
        for (int8_t x = 3; x <= 20; x++) {
            if (((uint8_t)(x + 8 - frame) & 3) < 2) tama_px(buf, x, 23);
        }

        int8_t b1y = 5 - (int8_t)(frame & 3);
        tama_rect(buf, 20, b1y, 3, 3);
        tama_clr(buf, 21, b1y + 1);

        int8_t b2y = 6 - (int8_t)((frame + 2) & 3);
        tama_rect(buf, 1, b2y, 2, 2);

    } else if (mood == MOOD_CALOR) {
        tama_hline(buf, 6, 9, 11);   tama_hline(buf, 7, 8, 12);
        tama_hline(buf, 14, 17, 11); tama_hline(buf, 15, 16, 12);

        tama_px(buf, 9, 16);
        tama_hline(buf, 10, 13, 15);
        tama_px(buf, 14, 16);

        int8_t yd = 2 + (int8_t)(frame & 3);
        tama_px(buf, 21, yd);
        tama_hline(buf, 20, 22, yd + 1);
        tama_px(buf, 21, yd + 2);

        int8_t hx = (frame & 1) ? 1 : 0;
        tama_px(buf, 2 + hx, 2);
        tama_px(buf, 3 - hx, 3);
        tama_px(buf, 2 + hx, 4);
        tama_px(buf, 3 - hx, 5);

    } else if (mood == MOOD_FRIO) {
        tama_hline(buf, 6 + sx, 7 + sx, 11);
        tama_px(buf, 8 + sx, 12);
        tama_hline(buf, 6 + sx, 7 + sx, 13);

        tama_hline(buf, 16 + sx, 17 + sx, 11);
        tama_px(buf, 15 + sx, 12);
        tama_hline(buf, 16 + sx, 17 + sx, 13);

        for (int8_t x = 8; x <= 15; x++) {
            tama_px(buf, x + sx, (x & 1) ? 15 : 16);
        }

    } else { // MOOD_NOITE
        tama_px(buf, 6, 12 + dy);  tama_hline(buf, 7, 8, 13 + dy);   tama_px(buf, 9, 12 + dy);
        tama_px(buf, 14, 12 + dy); tama_hline(buf, 15, 16, 13 + dy); tama_px(buf, 17, 12 + dy);

        if ((frame >> 1) & 1) {
            tama_rect(buf, 11, 15 + dy, 2, 2);
        } else {
            tama_hline(buf, 11, 12, 16 + dy);
        }

        int8_t zy = 1 + (int8_t)((frame >> 1) & 1);
        tama_hline(buf, 19, 22, zy);
        tama_px(buf, 21, zy + 1);
        tama_px(buf, 20, zy + 2);
        tama_hline(buf, 19, 22, zy + 3);
    }

    for (uint8_t p = 0; p < 6; p++) {
        oled_write_page(col_start, page_start + p, buf[p], 48);
    }
}