#ifndef AVR_CHIP_H
#define AVR_CHIP_H

/*
 * An AVR chip (plan-62-B): a CPU and the peripherals of a register layout.
 * The peripherals are avr8js ports (avr.h); the layouts are the register
 * addresses, bits, vectors and pins of the devices the simulator knows,
 * from Microchip's ATDF files (planning/plan-62-data).
 */

#include "avr.h"

enum {
    AVR_LAYOUT_PORTS = 5,
    AVR_LAYOUT_TIMERS = 5
};

typedef struct {
    char letter;             /* 'B', 'C', ... */
    AvrPortConfig config;
} AvrLayoutPort;

/*
 * A register layout. Every interrupt field holds the VECTOR NUMBER (the
 * ATDF's index: RESET is 0, INT0 is 1); avr_layout_scale turns them into
 * the word addresses avr8js's configs hold, for a chip's vector size.
 */
typedef struct AvrLayout {
    const char *name;
    size_t port_count;
    AvrLayoutPort ports[AVR_LAYOUT_PORTS];
    AvrClockConfig clock;
    size_t timer_count;
    AvrTimerConfig timers[AVR_LAYOUT_TIMERS];
} AvrLayout;

typedef struct AvrChip {
    AvrLayout layout;        /* scaled: interrupt fields are word addresses */
    AvrCpu *cpu;
    uint32_t base_hz;
    size_t port_count;
    AvrPort ports[AVR_LAYOUT_PORTS];
    AvrClock clock;
    size_t timer_count;
    AvrTimer timers[AVR_LAYOUT_TIMERS];
} AvrChip;

/* "atmega328": the ATmega48/88/168/328 (and A, P, PA) registers. NULL when unknown. */
const AvrLayout *avr_layout_find(const char *name);

/* Words per interrupt vector: 1 for a flash of 8192 bytes or less (the
   vector holds an RJMP), else 2 (a JMP). */
unsigned avr_vector_words(uint32_t flash_bytes);

/* Copies a layout with every interrupt field multiplied by the vector size
   of a chip with flash_bytes of flash. */
void avr_layout_scale(const AvrLayout *layout, uint32_t flash_bytes, AvrLayout *out);

/* clkpr_reset: CLKPR's value after reset (0; 3 for the internal oscillator
   divided by 8). NULL when out of memory. */
AvrChip *avr_chip_create(const AvrLayout *layout, uint32_t flash_bytes, uint32_t sram_bytes,
                         uint32_t eeprom_bytes, uint32_t base_hz, uint8_t clkpr_reset);
void avr_chip_destroy(AvrChip *chip);

/* A reset of the whole chip (the reset pin, a watchdog reset): every I/O
   register back to 0 with its pins released, the peripherals' state
   cleared, pc 0. Flash and SRAM are kept. */
void avr_chip_reset(AvrChip *chip);

/* One instruction, then avr8js's tick (a due clock event, a pending interrupt). */
void avr_chip_step(AvrChip *chip);

/* The port with this letter, or NULL. */
AvrPort *avr_chip_port(AvrChip *chip, char letter);

#endif
