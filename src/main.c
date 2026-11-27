#include "config.h"
#include "i2c.h"
#include "adc.h"
#include "oled.h"
#include "ui.h"
#include "ds3231.h"
#include "bh1750.h"
#include "bme280.h"

#include <util/delay.h>
#include <stdio.h>

int main(void) {
    char buf[24];
    rtc_time_t agora;
    uint16_t lux = 0;
    uint8_t frame = 0;

    i2c_init();
    adc_init();
    oled_init();
    oled_clear();
    oled_draw_divider(1);

    uint8_t rtc_ok = ds3231_init();
    bme280_init();
    uint8_t bh_ok  = bh1750_init();

#if ACERTAR_HORA
    if (rtc_ok) {
        uint8_t h = (uint8_t)((__TIME__[0] - '0') * 10 + (__TIME__[1] - '0'));
        uint8_t m = (uint8_t)((__TIME__[3] - '0') * 10 + (__TIME__[4] - '0'));
        uint8_t s = (uint8_t)((__TIME__[6] - '0') * 10 + (__TIME__[7] - '0'));
        ds3231_set_time(s, m, h, DIA_INICIAL, MES_INICIAL, ANO_INICIAL);
    }
#endif

#if !DEBUG_SCREEN
    ui_init_icons();
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

        // --- LEITURA DOS SENSORES ---
        bme280_data_t bme = { .temp_int = 22 };
        uint8_t bme_valid = bme280_read(&bme);
        uint8_t bh_valid  = (bh_ok && bh1750_read_lux(&lux));

        uint16_t soil_raw = adc_read_avg(0);
        uint8_t  soil_pct = soil_raw_to_pct(soil_raw);

#if DEBUG_SCREEN
        oled_set_cursor(0, 2);
        if (bme_valid) {
            snprintf(buf, sizeof(buf), "T:%d.%d*C H:%u%%", bme.temp_int, bme.temp_dec, bme.hum_pct);
        } else {
            snprintf(buf, sizeof(buf), "BME ERRO");
        }
        oled_print_padded(buf, 21);

        oled_set_cursor(0, 3);
        if (bme_valid && bh_valid) {
            snprintf(buf, sizeof(buf), "P:%uhPa L:%ulx", bme.press_hpa, lux);
        } else if (bh_valid) {
            snprintf(buf, sizeof(buf), "P:-- L:%ulx", lux);
        } else {
            snprintf(buf, sizeof(buf), "P:%uhPa L:ERRO", bme.press_hpa);
        }
        oled_print_padded(buf, 21);

        oled_set_cursor(0, 4);
        snprintf(buf, sizeof(buf), "SOLO RAW: %u", soil_raw);
        oled_print_padded(buf, 21);

        oled_set_cursor(0, 6);
        snprintf(buf, sizeof(buf), "SOLO PCT: %u%%", soil_pct);
        oled_print_padded(buf, 21);

        oled_set_cursor(0, 7);
        oled_print_padded("DEBUG MODE", 21);
#else
        oled_set_cursor(14, 2);
        if (bme_valid) {
            snprintf(buf, sizeof(buf), "%d.%d*C", bme.temp_int, bme.temp_dec);
            oled_print_padded(buf, 9);
        } else {
            oled_print_padded("ERR", 9);
        }

        oled_set_cursor(14, 4);
        if (bme_valid && bme.has_hum) {
            snprintf(buf, sizeof(buf), "%u%%", bme.hum_pct);
            oled_print_padded(buf, 9);
        } else {
            oled_print_padded("--", 9);
        }

        oled_set_cursor(14, 6);
        snprintf(buf, sizeof(buf), "%u%%", soil_pct);
        oled_print_padded(buf, 9);

        mood_t mood = MOOD_FELIZ;
        if (bh_valid && lux < LUX_NOITE) {
            mood = MOOD_NOITE;
        } else if (soil_pct <= SOIL_SEDE) {
            mood = MOOD_SEDE;
        } else if (soil_pct >= SOIL_ENCHARCADO) {
            mood = MOOD_ENCHARCADO;
        } else if (bme_valid && bme.temp_int >= TEMP_CALOR) {
            mood = MOOD_CALOR;
        } else if (bme_valid && bme.temp_int <= TEMP_FRIO) {
            mood = MOOD_FRIO;
        }

        ui_draw_face48(76, 2, mood, frame);
#endif

        frame = (frame + 1) & 0x07;
        _delay_ms(400);
    }
    return 0;
}