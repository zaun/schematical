#ifndef AVR_CHIP_H
#define AVR_CHIP_H

/*
 * An AVR chip (plan-62-B): a CPU and the peripherals of a register layout.
 * The peripherals are avr8js ports (avr.h), but for the USI (avr_usi.c);
 * the layouts are the register addresses, bits, vectors and pins of the
 * devices the simulator knows, from Microchip's ATDF files
 * (planning/plan-62-data) and, for the ATtiny25/45/85, their datasheet
 * (Atmel 2586).
 */

#include "avr.h"

enum {
    AVR_LAYOUT_PORTS = 5,
    AVR_LAYOUT_TIMERS = 5,
    AVR_LAYOUT_SERIALS = 2,   /* USARTs, SPIs and TWIs each */
    AVR_LAYOUT_ADC_PINS = 8
};

typedef struct {
    char letter;             /* 'B', 'C', ... */
    AvrPortConfig config;
} AvrLayoutPort;

/* A port pin by name: PD1 is {'D', 1}. */
typedef struct { char port; uint8_t bit; } AvrPinName;

/* The pins of the serial peripherals (the ATDF's signals), for the layer
   that puts their bytes on pins (plan-62-F). avr8js has no such tables: its
   serial peripherals stop at the byte. */
typedef struct { AvrPinName rxd, txd; } AvrUsartPins;
typedef struct { AvrPinName mosi, miso, sck; } AvrSpiPins;
typedef struct { AvrPinName sda, scl; } AvrTwiPins;

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
    size_t usart_count;
    AvrUsartConfig usarts[AVR_LAYOUT_SERIALS];
    size_t spi_count;
    AvrSpiConfig spis[AVR_LAYOUT_SERIALS];
    size_t twi_count;
    AvrTwiConfig twis[AVR_LAYOUT_SERIALS];
    AvrAdcConfig adc;
    AvrEepromConfig eeprom;      /* erase_cycles and write_cycles are set per chip */
    AvrWatchdogConfig watchdog;
    AvrUsartPins usart_pins[AVR_LAYOUT_SERIALS];
    AvrSpiPins spi_pins[AVR_LAYOUT_SERIALS];
    AvrTwiPins twi_pins[AVR_LAYOUT_SERIALS];
    /* The pins with a fixed job (port 0 = the chip has none): the clock
       input and output, the reset pin, and the ADC's channels. */
    AvrPinName xtal1, xtal2, reset;
    AvrPinName adc_pins[AVR_LAYOUT_ADC_PINS];
    /* Where the SRAM starts in the data space: 0x100, or 0x60 on an
       ATtiny25/45/85, which has no extended I/O. */
    uint16_t sram_start;
    int has_tiny_timer1;         /* the ATtiny25/45/85's Timer/Counter1 */
    AvrTinyTimer1Config tiny_timer1;
    int has_usi;
    AvrUsiConfig usi;
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
    AvrTinyTimer1 tiny_timer1;   /* when layout.has_tiny_timer1 */
    AvrUsi usi;                  /* when layout.has_usi */
    size_t usart_count;
    AvrUsart usarts[AVR_LAYOUT_SERIALS];
    size_t spi_count;
    AvrSpi spis[AVR_LAYOUT_SERIALS];
    size_t twi_count;
    AvrTwi twis[AVR_LAYOUT_SERIALS];
    AvrAdc adc;
    AvrEeprom eeprom;
    AvrWatchdog watchdog;
    /* Runs after avr_chip_reset when the chip reset itself (the watchdog). */
    AvrCallback after_reset;
} AvrChip;

/* A layout by name, NULL when unknown:
     "atmega328"    the ATmega48/88/168/328 and their A, P and PA versions
     "atmega168pb"  the ATmega48PB/88PB/168PB: the same plus port E (PE0-PE3)
     "atmega328pb"  the ATmega328PB: port E with PCINT3, timers 3 and 4,
                    USART1, SPI1 and TWI1
     "attiny85"     the ATtiny25/45/85: port B (6 pins), Timer0, the 8-bit
                    Timer1 with its own prescaler, the USI, a 4-channel ADC */
const AvrLayout *avr_layout_find(const char *name);

/* Words per interrupt vector: 1 for a flash of 8192 bytes or less (the
   vector holds an RJMP), else 2 (a JMP). */
unsigned avr_vector_words(uint32_t flash_bytes);

/* Copies a layout with every interrupt field multiplied by the vector size
   of a chip with flash_bytes of flash. */
void avr_layout_scale(const AvrLayout *layout, uint32_t flash_bytes, AvrLayout *out);

/* sram_bytes: the SRAM alone (the data space is the layout's sram_start
   more). clkpr_reset: CLKPR's value after reset (0; 3 for the internal
   oscillator divided by 8). NULL when out of memory. */
AvrChip *avr_chip_create(const AvrLayout *layout, uint32_t flash_bytes, uint32_t sram_bytes,
                         uint32_t eeprom_bytes, uint32_t base_hz, uint8_t clkpr_reset);
void avr_chip_destroy(AvrChip *chip);

/* A reset of the whole chip (the reset pin, a watchdog reset): every I/O
   register back to 0 with its pins released, the peripherals' state
   cleared, pc 0. Flash and SRAM are kept. */
void avr_chip_reset(AvrChip *chip);

/* One instruction, then avr8js's tick (a due clock event, a pending interrupt). */
void avr_chip_step(AvrChip *chip);

/* The EEPROM's erase time and its write time, in CPU cycles, at a clock of
   hz: 1.8 ms each (avr8js's 28800 cycles at 16 MHz). */
uint32_t avr_eeprom_cycles(uint32_t hz);

/* Tells the chip its clock's frequency once it is known (plan-62-E): the
   EEPROM's write times and the watchdog's timeout follow it. */
void avr_chip_set_base_hz(AvrChip *chip, uint32_t hz);

/* The port with this letter, or NULL. */
AvrPort *avr_chip_port(AvrChip *chip, char letter);

#endif
