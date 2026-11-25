/*
 * AVR chips: the register layouts and the assembly of a CPU with its
 * peripherals (plan-62-B). The peripherals are C ports of avr8js (avr.h);
 * the "atmega328" tables equal avr8js's own ATmega328P configs, which
 * avr_periph_test checks against avr8js (`layout atmega328 equal`).
 */

#include <stdlib.h>
#include <string.h>

#include "avr_chip.h"

/* ---- Vector numbers of the ATmega48/88/168/328 (ATmega328P.atdf) ----------- */

enum {
    V_INT0 = 1, V_INT1 = 2, V_PCINT0 = 3, V_PCINT1 = 4, V_PCINT2 = 5, V_WDT = 6,
    V_TIMER2_COMPA = 7, V_TIMER2_COMPB = 8, V_TIMER2_OVF = 9,
    V_TIMER1_CAPT = 10, V_TIMER1_COMPA = 11, V_TIMER1_COMPB = 12, V_TIMER1_OVF = 13,
    V_TIMER0_COMPA = 14, V_TIMER0_COMPB = 15, V_TIMER0_OVF = 16,
    V_SPI_STC = 17, V_USART_RX = 18, V_USART_UDRE = 19, V_USART_TX = 20, V_ADC = 21,
    V_EE_READY = 22, V_ANALOG_COMP = 23, V_TWI = 24, V_SPM_READY = 25
};

/* ---- Register addresses (data space) --------------------------------------- */

enum {
    PINB = 0x23, DDRB = 0x24, PORTB = 0x25,
    PINC = 0x26, DDRC = 0x27, PORTC = 0x28,
    PIND = 0x29, DDRD = 0x2a, PORTD = 0x2b,
    PCIFR = 0x3b, EIFR = 0x3c, EIMSK = 0x3d,
    CLKPR = 0x61, PCICR = 0x68, EICRA = 0x69,
    PCMSK0 = 0x6b, PCMSK1 = 0x6c, PCMSK2 = 0x6d,
    EECR = 0x3f, EEDR = 0x40, EEARL = 0x41, EEARH = 0x42,
    SPCR = 0x4c, SPSR = 0x4d, SPDR = 0x4e,
    MCUSR = 0x54, WDTCSR = 0x60,
    ADCL = 0x78, ADCH = 0x79, ADCSRA = 0x7a, ADCSRB = 0x7b, ADMUX = 0x7c, DIDR0 = 0x7e,
    TWBR = 0xb8, TWSR = 0xb9, TWAR = 0xba, TWDR = 0xbb, TWCR = 0xbc, TWAMR = 0xbd,
    UCSR0A = 0xc0, UCSR0B = 0xc1, UCSR0C = 0xc2, UBRR0L = 0xc4, UBRR0H = 0xc5, UDR0 = 0xc6
};

#define PIN_CHANGE(PCIE_BIT, PCMSK_ADDR, VECTOR) \
    1, {PCIE_BIT, PCICR, PCIFR, PCMSK_ADDR, VECTOR, 0xff, 0}
#define EXTERNAL(ISC, INDEX, VECTOR) {1, EICRA, EIMSK, EIFR, ISC, INDEX, VECTOR}
#define NO_EXTERNAL {0, 0, 0, 0, 0, 0, 0}

#define TIMER01_DIVIDERS {0, 1, 8, 64, 256, 1024, 0, 0}   /* 6, 7: the external clock pin */
#define TIMER2_DIVIDERS {0, 1, 8, 32, 64, 128, 256, 1024}
/* TIFR bits TOV, OCFA, OCFB, OCFC; TIMSK bits TOIE, OCIEA, OCIEB, OCIEC. */
#define TIMER_BITS 1, 2, 4, 0, 1, 2, 4, 0

#define PORT_B {'B', {PINB, DDRB, PORTB, PIN_CHANGE(0, PCMSK0, V_PCINT0), {NO_EXTERNAL}}}
#define PORT_C {'C', {PINC, DDRC, PORTC, PIN_CHANGE(1, PCMSK1, V_PCINT1), {NO_EXTERNAL}}}
#define PORT_D {'D', {PIND, DDRD, PORTD, PIN_CHANGE(2, PCMSK2, V_PCINT2), \
                      {NO_EXTERNAL, NO_EXTERNAL, EXTERNAL(0, 0, V_INT0), EXTERNAL(2, 1, V_INT1)}}}

#define TIMER0 { \
    8, TIMER01_DIVIDERS, \
    0, V_TIMER0_COMPA, V_TIMER0_COMPB, 0, V_TIMER0_OVF, \
    /* TIFR OCRA OCRB OCRC ICR TCNT TCCRA TCCRB TCCRC TIMSK */ \
    0x35, 0x47, 0x48, 0, 0, 0x46, 0x44, 0x45, 0, 0x6e, \
    TIMER_BITS, \
    PORTD, 6, PORTD, 5, 0, 0, /* OC0A PD6, OC0B PD5 */ \
    PORTD, 4 /* T0 PD4 */ }

#define TIMER1 { \
    16, TIMER01_DIVIDERS, \
    V_TIMER1_CAPT, V_TIMER1_COMPA, V_TIMER1_COMPB, 0, V_TIMER1_OVF, \
    0x36, 0x88, 0x8a, 0, 0x86, 0x84, 0x80, 0x81, 0x82, 0x6f, \
    TIMER_BITS, \
    PORTB, 1, PORTB, 2, 0, 0, /* OC1A PB1, OC1B PB2 */ \
    PORTD, 5 /* T1 PD5 */ }

#define TIMER2 { \
    8, TIMER2_DIVIDERS, \
    0, V_TIMER2_COMPA, V_TIMER2_COMPB, 0, V_TIMER2_OVF, \
    0x37, 0xb3, 0xb4, 0, 0, 0xb2, 0xb0, 0xb1, 0, 0x70, \
    TIMER_BITS, \
    PORTB, 3, PORTD, 3, 0, 0, /* OC2A PB3, OC2B PD3 */ \
    0, 0 }

#define USART0 {V_USART_RX, V_USART_UDRE, V_USART_TX, UCSR0A, UCSR0B, UCSR0C, UBRR0L, UBRR0H, UDR0}
#define SPI0 {V_SPI_STC, SPCR, SPSR, SPDR}
#define TWI0 {V_TWI, TWBR, TWSR, TWAR, TWDR, TWCR, TWAMR}

#define SINGLE(CHANNEL) {1, AVR_ADC_SINGLE_ENDED, CHANNEL, 0, 0, 0, 0}
#define CONSTANT(VOLTS) {1, AVR_ADC_CONSTANT, 0, VOLTS, 0, 0, 0}
#define NO_INPUT {0, AVR_ADC_CONSTANT, 0, 0, 0, 0, 0}
/* The ATmega328's ADC: channels 0-7, the temperature sensor at 8, the
   1.1 V band gap at 14, ground at 15; references AREF, AVCC, -, 1.1 V. */
#define ADC_328 { \
    ADMUX, ADCSRA, ADCSRB, ADCL, ADCH, DIDR0, V_ADC, 8, 0xf, \
    {SINGLE(0), SINGLE(1), SINGLE(2), SINGLE(3), SINGLE(4), SINGLE(5), SINGLE(6), SINGLE(7), \
     {1, AVR_ADC_TEMPERATURE, 0, 0, 0, 0, 0}, NO_INPUT, NO_INPUT, NO_INPUT, NO_INPUT, NO_INPUT, \
     CONSTANT(1.1), CONSTANT(0)}, \
    {AVR_ADC_AREF, AVR_ADC_AVCC, AVR_ADC_RESERVED, AVR_ADC_INTERNAL_1V1}, 4}

#define EEPROM_328 {V_EE_READY, EECR, EEDR, EEARL, EEARH, 0, 0}
#define WATCHDOG_328 {V_WDT, MCUSR, WDTCSR}

/* ---- The PB devices (ATmega48PB/88PB/168PB.atdf, ATmega328PB.atdf) ---------- */

enum {
    V_PCINT3 = 27, V_USART1_RX = 28, V_USART1_UDRE = 29, V_USART1_TX = 30,
    V_TIMER3_CAPT = 32, V_TIMER3_COMPA = 33, V_TIMER3_COMPB = 34, V_TIMER3_OVF = 35,
    V_SPI1_STC = 39, V_TWI1 = 40,
    V_TIMER4_CAPT = 41, V_TIMER4_COMPA = 42, V_TIMER4_COMPB = 43, V_TIMER4_OVF = 44
};

enum { PINE = 0x2c, DDRE = 0x2d, PORTE = 0x2e, PCMSK3 = 0x73 };

/* Port E has four pins, PE0-PE3. On the 48PB/88PB/168PB it has no interrupt;
   on the 328PB it is PCINT24-27 (PCIE3, PCMSK3). */
#define PORT_E {'E', {PINE, DDRE, PORTE, 0, {0, 0, 0, 0, 0, 0, 0}, {NO_EXTERNAL}}}
#define PORT_E_PCINT3 {'E', {PINE, DDRE, PORTE, 1, {3, PCICR, PCIFR, PCMSK3, V_PCINT3, 0x0f, 0}, \
                             {NO_EXTERNAL}}}

/* Timers 3 and 4 of the 328PB: timer 1's shape. Their external clock pins
   (T3, T4) are not modeled: clock select 6 and 7 stop the timer. */
#define TIMER34_DIVIDERS {0, 1, 8, 64, 256, 1024, 0, 0}
#define TIMER3 { \
    16, TIMER34_DIVIDERS, \
    V_TIMER3_CAPT, V_TIMER3_COMPA, V_TIMER3_COMPB, 0, V_TIMER3_OVF, \
    /* TIFR OCRA OCRB OCRC ICR TCNT TCCRA TCCRB TCCRC TIMSK */ \
    0x38, 0x98, 0x9a, 0, 0x96, 0x94, 0x90, 0x91, 0x92, 0x71, \
    TIMER_BITS, \
    PORTD, 0, PORTD, 2, 0, 0, /* OC3A PD0, OC3B PD2 */ \
    0, 0 }
#define TIMER4 { \
    16, TIMER34_DIVIDERS, \
    V_TIMER4_CAPT, V_TIMER4_COMPA, V_TIMER4_COMPB, 0, V_TIMER4_OVF, \
    0x39, 0xa8, 0xaa, 0, 0xa6, 0xa4, 0xa0, 0xa1, 0xa2, 0x72, \
    TIMER_BITS, \
    PORTD, 1, PORTD, 2, 0, 0, /* OC4A PD1, OC4B PD2 */ \
    0, 0 }

#define USART1 {V_USART1_RX, V_USART1_UDRE, V_USART1_TX, 0xc8, 0xc9, 0xca, 0xcc, 0xcd, 0xce}
#define SPI1 {V_SPI1_STC, 0xac, 0xad, 0xae}
#define TWI1 {V_TWI1, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd}

static const AvrLayout LAYOUTS[] = {
    {
        "atmega168pb",
        4, {PORT_B, PORT_C, PORT_D, PORT_E},
        {CLKPR},
        3, {TIMER0, TIMER1, TIMER2},
        1, {USART0},
        1, {SPI0},
        1, {TWI0},
        ADC_328,
        EEPROM_328,
        WATCHDOG_328,
    },
    {
        "atmega328pb",
        4, {PORT_B, PORT_C, PORT_D, PORT_E_PCINT3},
        {CLKPR},
        5, {TIMER0, TIMER1, TIMER2, TIMER3, TIMER4},
        2, {USART0, USART1},
        2, {SPI0, SPI1},
        2, {TWI0, TWI1},
        ADC_328,
        EEPROM_328,
        WATCHDOG_328,
    },
    {
        "atmega328",
        3, {PORT_B, PORT_C, PORT_D},
        {CLKPR},
        3, {TIMER0, TIMER1, TIMER2},
        1, {USART0},
        1, {SPI0},
        1, {TWI0},
        ADC_328,
        EEPROM_328,
        WATCHDOG_328,
    },
};

const AvrLayout *avr_layout_find(const char *name) {
    size_t i;

    for (i = 0; i < sizeof LAYOUTS / sizeof LAYOUTS[0]; i++) {
        if (strcmp(LAYOUTS[i].name, name) == 0) {
            return &LAYOUTS[i];
        }
    }
    return NULL;
}

unsigned avr_vector_words(uint32_t flash_bytes) {
    return flash_bytes <= 8192 ? 1 : 2;
}

void avr_layout_scale(const AvrLayout *layout, uint32_t flash_bytes, AvrLayout *out) {
    const uint8_t words = (uint8_t)avr_vector_words(flash_bytes);
    size_t i;
    int pin;

    *out = *layout;
    for (i = 0; i < out->port_count; i++) {
        AvrPortConfig *port = &out->ports[i].config;

        port->pin_change.pin_change_interrupt *= words;
        for (pin = 0; pin < 8; pin++) {
            port->external[pin].interrupt *= words;
        }
    }
    for (i = 0; i < out->timer_count; i++) {
        AvrTimerConfig *timer = &out->timers[i];

        timer->capture_interrupt *= words;
        timer->comp_a_interrupt *= words;
        timer->comp_b_interrupt *= words;
        timer->comp_c_interrupt *= words;
        timer->ovf_interrupt *= words;
    }
    for (i = 0; i < out->usart_count; i++) {
        out->usarts[i].rx_complete_interrupt *= words;
        out->usarts[i].data_register_empty_interrupt *= words;
        out->usarts[i].tx_complete_interrupt *= words;
    }
    for (i = 0; i < out->spi_count; i++) {
        out->spis[i].spi_interrupt *= words;
    }
    for (i = 0; i < out->twi_count; i++) {
        out->twis[i].twi_interrupt *= words;
    }
    out->adc.adc_interrupt *= words;
    out->eeprom.eeprom_ready_interrupt *= words;
    out->watchdog.watchdog_interrupt *= words;
}

uint32_t avr_eeprom_cycles(uint32_t hz) {
    return (uint32_t)((double)hz * 0.0018 + 0.5);
}

static void chip_reset_event(void *ctx) {
    AvrChip *chip = ctx;

    avr_chip_reset(chip);
    if (chip->after_reset.fn != NULL) {
        chip->after_reset.fn(chip->after_reset.ctx);
    }
}

void avr_chip_set_base_hz(AvrChip *chip, uint32_t hz) {
    chip->base_hz = hz;
    chip->clock.base_hz = hz;
    chip->eeprom.config.erase_cycles = avr_eeprom_cycles(hz);
    chip->eeprom.config.write_cycles = avr_eeprom_cycles(hz);
}

AvrChip *avr_chip_create(const AvrLayout *layout, uint32_t flash_bytes, uint32_t sram_bytes,
                         uint32_t eeprom_bytes, uint32_t base_hz, uint8_t clkpr_reset) {
    AvrChip *chip = calloc(1, sizeof(*chip));
    size_t i;

    if (chip == NULL) {
        return NULL;
    }
    avr_layout_scale(layout, flash_bytes, &chip->layout);
    chip->base_hz = base_hz;
    chip->cpu = avr_cpu_create(flash_bytes, sram_bytes);
    if (chip->cpu == NULL) {
        free(chip);
        return NULL;
    }
    /* One order on both sides (oracle.js run builds avr8js's objects in
       it too): ports, clock, timers, USARTs, SPIs, TWIs, ADC, EEPROM,
       watchdog. A later peripheral's hook on a shared register replaces an
       earlier one. */
    chip->port_count = chip->layout.port_count;
    for (i = 0; i < chip->port_count; i++) {
        avr_port_init(&chip->ports[i], chip->cpu, &chip->layout.ports[i].config);
    }
    avr_clock_init(&chip->clock, chip->cpu, base_hz, &chip->layout.clock, clkpr_reset);
    chip->timer_count = chip->layout.timer_count;
    for (i = 0; i < chip->timer_count; i++) {
        avr_timer_init(&chip->timers[i], chip->cpu, &chip->layout.timers[i]);
    }
    chip->usart_count = chip->layout.usart_count;
    for (i = 0; i < chip->usart_count; i++) {
        avr_usart_init(&chip->usarts[i], chip->cpu, &chip->layout.usarts[i], base_hz);
    }
    chip->spi_count = chip->layout.spi_count;
    for (i = 0; i < chip->spi_count; i++) {
        avr_spi_init(&chip->spis[i], chip->cpu, &chip->layout.spis[i], base_hz);
    }
    chip->twi_count = chip->layout.twi_count;
    for (i = 0; i < chip->twi_count; i++) {
        avr_twi_init(&chip->twis[i], chip->cpu, &chip->layout.twis[i], base_hz);
    }
    avr_adc_init(&chip->adc, chip->cpu, &chip->layout.adc);
    chip->layout.eeprom.erase_cycles = avr_eeprom_cycles(base_hz);
    chip->layout.eeprom.write_cycles = avr_eeprom_cycles(base_hz);
    if (avr_eeprom_init(&chip->eeprom, chip->cpu, eeprom_bytes, &chip->layout.eeprom) != 0) {
        avr_cpu_destroy(chip->cpu);
        free(chip);
        return NULL;
    }
    avr_watchdog_init(&chip->watchdog, chip->cpu, &chip->layout.watchdog, &chip->clock);
    chip->cpu->on_reset.fn = chip_reset_event;
    chip->cpu->on_reset.ctx = chip;
    return chip;
}

void avr_chip_destroy(AvrChip *chip) {
    if (chip == NULL) {
        return;
    }
    avr_eeprom_free(&chip->eeprom);
    avr_cpu_destroy(chip->cpu);
    free(chip);
}

AvrPort *avr_chip_port(AvrChip *chip, char letter) {
    size_t i;

    for (i = 0; i < chip->port_count; i++) {
        if (chip->layout.ports[i].letter == letter) {
            return &chip->ports[i];
        }
    }
    return NULL;
}

void avr_chip_reset(AvrChip *chip) {
    AvrCpu *cpu = chip->cpu;
    size_t i;

    /* Through the hooks first, so compare outputs let go of their pins and
       every port tells its listener the pins are inputs again. */
    for (i = 0; i < chip->timer_count; i++) {
        const AvrTimerConfig *timer = &chip->layout.timers[i];

        avr_write_data(cpu, timer->TCCRA, 0, 0xff);
        avr_write_data(cpu, timer->TCCRB, 0, 0xff);
    }
    for (i = 0; i < chip->port_count; i++) {
        const AvrPortConfig *port = &chip->layout.ports[i].config;

        avr_write_data(cpu, port->DDR, 0, 0xff);
        avr_write_data(cpu, port->PORT, 0, 0xff);
    }
    /* Every I/O register to 0 (SREG and SP included; the CPU reset sets SP). */
    memset(cpu->data + 32, 0, AVR_REGISTER_SPACE - 32);
    for (i = 0; i < chip->port_count; i++) {
        /* PINx keeps showing what the outside drives. */
        avr_port_set_pin(&chip->ports[i], 0, chip->ports[i].pin_value & 1);
    }
    for (i = 0; i < chip->timer_count; i++) {
        avr_timer_reset(&chip->timers[i]);
    }
    for (i = 0; i < chip->usart_count; i++) {
        avr_usart_reset(&chip->usarts[i]);
    }
    for (i = 0; i < chip->spi_count; i++) {
        avr_spi_reset(&chip->spis[i]);
    }
    for (i = 0; i < chip->twi_count; i++) {
        avr_twi_reset(&chip->twis[i]);
    }
    avr_adc_reset(&chip->adc);
    avr_eeprom_reset(&chip->eeprom);
    avr_watchdog_reset(&chip->watchdog);
    avr_cpu_reset(cpu);
    cpu->cycles = 0;
    avr_clock_reset(&chip->clock);
}

void avr_chip_step(AvrChip *chip) {
    avr_instruction(chip->cpu);
    avr_cpu_tick(chip->cpu);
}
