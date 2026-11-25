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
    PCMSK0 = 0x6b, PCMSK1 = 0x6c, PCMSK2 = 0x6d
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

static const AvrLayout LAYOUTS[] = {
    {
        "atmega328",
        3, {PORT_B, PORT_C, PORT_D},
        {CLKPR},
        3, {TIMER0, TIMER1, TIMER2},
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
}

AvrChip *avr_chip_create(const AvrLayout *layout, uint32_t flash_bytes, uint32_t sram_bytes,
                         uint32_t eeprom_bytes, uint32_t base_hz, uint8_t clkpr_reset) {
    AvrChip *chip = calloc(1, sizeof(*chip));
    size_t i;

    (void)eeprom_bytes;
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
    /* The order avr8js's users build a chip in: ports, clock, timers. A
       later peripheral's hook on a shared register replaces an earlier one. */
    chip->port_count = chip->layout.port_count;
    for (i = 0; i < chip->port_count; i++) {
        avr_port_init(&chip->ports[i], chip->cpu, &chip->layout.ports[i].config);
    }
    avr_clock_init(&chip->clock, chip->cpu, base_hz, &chip->layout.clock, clkpr_reset);
    chip->timer_count = chip->layout.timer_count;
    for (i = 0; i < chip->timer_count; i++) {
        avr_timer_init(&chip->timers[i], chip->cpu, &chip->layout.timers[i]);
    }
    return chip;
}

void avr_chip_destroy(AvrChip *chip) {
    if (chip == NULL) {
        return;
    }
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
    avr_cpu_reset(cpu);
    cpu->cycles = 0;
    avr_clock_reset(&chip->clock);
}

void avr_chip_step(AvrChip *chip) {
    avr_instruction(chip->cpu);
    avr_cpu_tick(chip->cpu);
}
