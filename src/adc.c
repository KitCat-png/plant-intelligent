#include "adc.h"
#include "config.h"
#include <avr/io.h>

void adc_init(void) {
    ADMUX  = (1 << REFS0); // Referência AVCC (5V) + 100nF em AREF
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // 125 kHz
    DIDR0 |= (1 << ADC0D); // Desativa buffer digital em PC0 (menos ruído)

    // Leitura de descarte para estabilizar o ADC
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));
}

uint16_t adc_read_avg(uint8_t channel) {
    ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);
    uint16_t sum = 0;
    for (uint8_t i = 0; i < 16; i++) {
        ADCSRA |= (1 << ADSC);
        while (ADCSRA & (1 << ADSC));
        sum += ADC;
    }
    return (sum >> 4);
}

uint8_t soil_raw_to_pct(uint16_t raw) {
    if (SOIL_DRY <= SOIL_WET) return 0;
    if (raw >= SOIL_DRY) return 0;
    if (raw <= SOIL_WET) return 100;
    return (uint8_t)(((uint32_t)(SOIL_DRY - raw) * 100UL) / (uint32_t)(SOIL_DRY - SOIL_WET));
}