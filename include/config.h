#ifndef CONFIG_H
#define CONFIG_H

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

// Endereços I2C (formato 8-bit com bit R/W a 0)
#define OLED_ADDR       (0x3C << 1)
#define DS3231_ADDR     (0x68 << 1)

// 1 = Ecrã de calibração (RAW, lux, pressão) | 0 = UI final minimalista
#define DEBUG_SCREEN    0

// Mantém a 0 para preservar a hora guardada no DS3231
#define ACERTAR_HORA    0
#define DIA_INICIAL     21
#define MES_INICIAL     11
#define ANO_INICIAL     26

// Calibração do sensor capacitivo v1.2 (PC0 / ADC0)
#define SOIL_DRY        786   // Ao ar livre (0%)
#define SOIL_WET        413   // Em água até à linha branca (100%)

// Limiares de estados do Tamagotchi
#define LUX_NOITE       20
#define SOIL_SEDE       30    // <= 30% -> sede
#define SOIL_ENCHARCADO 85    // >= 85% -> excesso de água
#define TEMP_CALOR      27
#define TEMP_FRIO       17

#endif