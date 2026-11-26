/*
 * mcu_cov_test (plan-68-S): the simulated AVR on the engine (mcu_avr.c,
 * mcu_avr_serial.c) where mcu_test, avr_serial_test and avr_tiny_test do
 * not go, with firmware assembled here (a few words each). TESTAVR is
 * mcu_fixture.h's ATmega328P on the p28 pinout; the internal clock is
 * 8 MHz / 8 = 1 MHz. ATmega328P datasheet sections:
 *
 *   TWI     22.9.2 TWEN: the TWI takes SDA/SCL (open drain, pull-ups from
 *           PORTC); clearing it ends a transfer and gives the pins back
 *   SPI     23.5 a master's SCK/MOSI; SPE cleared mid-byte stops the clock.
 *           A slave (MSTR 0) is not modeled: the byte ends with 0 after its
 *           time (mcu_avr_serial.c)
 *   USART   20.6 frames go out LSB first, back to back; nothing with TXEN 0;
 *           20.7.1 a start bit that is high at its middle is a glitch
 *   reset   10.9.1 a watchdog reset (WDRF), on the internal clock or a
 *           crystal; RESET held from power-up keeps the chip in reset
 *   clock   8.12.2 CLKPR /2 halves the CPU clock of an external crystal
 *   pins    a pinout without PC6 (no reset pin) or without PB5 (no SCK)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "circuit.h"
#include "cpu_state.h"
#include "mcu_fixture.h"
#include "parts.h"
#include "read_file.h"
#include "s68_mutate.h"

static PartLibrary *library;
static int failures;

#include "avr_rig.h"

static void check(int ok, const char *name) {
    printf("%s %s\n", ok ? "ok" : "FAIL:", name);
    if (!ok) {
        failures++;
    }
}

/* ---- A small assembler ------------------------------------------------------------ */

typedef struct {
    uint16_t w[128];
    size_t n;
} Fw;

enum {
    DDRB = 0x24, PORTB = 0x25, DDRC = 0x27, DDRD = 0x2A, PORTD = 0x2B, SPCR = 0x4C, SPSR = 0x4D,
    SPDR = 0x4E, MCUSR = 0x54, WDTCSR = 0x60, CLKPR = 0x61, TWBR = 0xB8, TWCR = 0xBC,
    UCSR0A = 0xC0, UCSR0B = 0xC1, UDR0 = 0xC6
};

static void op(Fw *f, uint16_t w) { f->w[f->n++] = w; }
static uint16_t ldi(int d, int k) {
    return (uint16_t)(0xE000 | ((k & 0xF0) << 4) | ((d - 16) << 4) | (k & 0x0F));
}
static void sts(Fw *f, uint16_t k, int r) { op(f, (uint16_t)(0x9200 | (r << 4))); op(f, k); }
static void lds(Fw *f, int d, uint16_t k) { op(f, (uint16_t)(0x9000 | (d << 4))); op(f, k); }
/* addr = value, through r16. */
static void set(Fw *f, uint16_t addr, int value) { op(f, ldi(16, value)); sts(f, addr, 16); }
/* 3n cycles: LDI r20, n; DEC r20; BRNE .-2 */
static void delay(Fw *f, int n) { op(f, ldi(20, n)); op(f, 0x954A); op(f, 0xF7F1); }
/* Until bit b of addr is 1: LDS r17, addr; SBRS r17, b; RJMP .-4 */
static void wait_bit(Fw *f, uint16_t addr, int b) {
    lds(f, 17, addr);
    op(f, (uint16_t)(0xFE00 | (17 << 4) | b));
    op(f, 0xCFFC);
}
static void halt(Fw *f) { op(f, 0xCFFF); }   /* RJMP . */

static void load(Rig *r, const Fw *f) {
    uint32_t image[256];
    size_t i;

    for (i = 0; i < f->n; i++) {
        image[2 * i] = f->w[i] & 0xff;
        image[2 * i + 1] = f->w[i] >> 8;
    }
    if (circuit_memory_load(r->c, r->avr, 0, image, (uint32_t)(2 * f->n)) != 0) {
        check(0, "firmware loads");
    }
}

static CircuitValue level(Rig *r, const char *pin) {
    return circuit_net_value(r->c, rig_net(r, pin));
}

/* Advances until the pin's net has the value; 0 when it did not within limit. */
static int wait_for(Rig *r, const char *pin, CircuitValue value, uint64_t limit) {
    uint64_t end = circuit_time(r->c) + limit;

    while (circuit_time(r->c) < end) {
        if (level(r, pin) == value) return 1;
        circuit_advance(r->c, US);
    }
    return level(r, pin) == value;
}

/* ---- TWI ----------------------------------------------------------------------------- */

static void twi_abandon(void) {
    Rig r;
    Fw f = {{0}, 0};
    uint64_t changes;
    int seen_low, seen_released, seen_low_again;

    set(&f, DDRC, 0x10);                  /* PC4 (SDA) a port output, low */
    delay(&f, 250);
    set(&f, TWBR, 0xFF);
    set(&f, TWCR, 0x04);                  /* TWEN: SDA released, PORTC4 0: no pull-up */
    delay(&f, 250);
    set(&f, TWCR, 0xA4);                  /* START: SCL never reads high (no pull-up) */
    delay(&f, 5);
    set(&f, TWCR, 0x00);                  /* TWEN 0: the transfer is dropped */
    halt(&f);
    rig_new(&r, "TESTAVR", NULL);
    load(&r, &f);
    if (rig_start(&r) != 0) return;
    seen_low = wait_for(&r, "PC4", CIRCUIT_0, 3 * MS);
    seen_released = wait_for(&r, "PC4", CIRCUIT_Z, MS);
    seen_low_again = wait_for(&r, "PC4", CIRCUIT_0, 3 * MS);
    changes = circuit_net_changes(r.c, rig_net(&r, "PC4")) + circuit_net_changes(r.c, rig_net(&r, "PC5"));
    circuit_advance(r.c, 3 * MS);
    check(seen_low && seen_released && seen_low_again,
          "twi: TWEN releases SDA (Z with PORTC4 0); TWEN 0 gives it back to the port (0)");
    check(level(&r, "PC5") == CIRCUIT_Z &&
              circuit_net_changes(r.c, rig_net(&r, "PC4")) + circuit_net_changes(r.c, rig_net(&r, "PC5")) ==
                  changes,
          "twi: the dropped START does nothing more on SDA or SCL");
    circuit_destroy(r.c);
}

/* ---- SPI ------------------------------------------------------------------------------ */

static void spi_abort(void) {
    Rig r;
    Fw f = {{0}, 0};
    uint64_t changes;

    set(&f, DDRB, 0x28);                  /* SCK (PB5), MOSI (PB3) outputs */
    set(&f, SPCR, 0x53);                  /* SPE, MSTR, clk/128: 64 us a half bit */
    set(&f, SPDR, 0xAA);                  /* MOSI 1: the first bit */
    op(&f, 0);
    op(&f, 0);
    set(&f, SPCR, 0x00);                  /* SPE 0 before the first SCK edge */
    halt(&f);
    rig_new(&r, "TESTAVR", NULL);
    load(&r, &f);
    if (rig_start(&r) != 0) return;
    rig_to(&r, 2 * MS);
    changes = circuit_net_changes(r.c, rig_net(&r, "PB5"));
    circuit_advance(r.c, 3 * MS);
    check(level(&r, "PB5") == CIRCUIT_0 && level(&r, "PB3") == CIRCUIT_0 &&
              circuit_net_changes(r.c, rig_net(&r, "PB5")) == changes,
          "spi: SPE cleared mid-byte: SCK and MOSI back to the port, no more clock");
    circuit_destroy(r.c);
}

static void spi_slave(void) {
    Rig r;
    Fw f = {{0}, 0};
    int value;

    set(&f, DDRB, 0x01);
    set(&f, DDRD, 0xFC);                  /* PD2-PD7 */
    set(&f, PORTD, 0xFC);
    set(&f, SPCR, 0x40);                  /* SPE, a slave */
    set(&f, SPDR, 0x55);
    wait_bit(&f, SPSR, 7);                /* SPIF */
    lds(&f, 18, SPDR);
    sts(&f, PORTD, 18);
    set(&f, PORTB, 0x01);
    halt(&f);
    rig_new(&r, "TESTAVR", NULL);
    load(&r, &f);
    if (rig_start(&r) != 0) return;
    rig_to(&r, 3 * MS);
    value = level(&r, "PD7") == CIRCUIT_0 && level(&r, "PD2") == CIRCUIT_0;
    check(level(&r, "PB0") == CIRCUIT_1 && value,
          "spi: a slave's byte ends (SPIF) with 0 in SPDR");
    circuit_destroy(r.c);
}

/* ---- USART ----------------------------------------------------------------------------- */

static void usart_tx_disabled(void) {
    Rig r;
    Fw f = {{0}, 0};

    set(&f, UDR0, 'A');                   /* TXEN 0: nothing goes out */
    delay(&f, 50);
    set(&f, UCSR0B, 0x08);                /* TXEN: TXD idles high */
    halt(&f);
    rig_new(&r, "TESTAVR", NULL);
    load(&r, &f);
    if (rig_start(&r) != 0) return;
    rig_to(&r, 5 * MS);
    check(level(&r, "PD1") == CIRCUIT_1 && circuit_net_changes(r.c, rig_net(&r, "PD1")) == 1,
          "usart: a byte written with TXEN 0 is not sent later");
    circuit_destroy(r.c);
}

static void usart_two_frames(void) {
    static const int WANT[20] = {0, 1, 0, 0, 0, 0, 0, 1, 0, 1,    /* 'A' 0x41, LSB first */
                                 0, 0, 1, 0, 0, 0, 0, 1, 0, 1};   /* 'B' 0x42 */
    Rig r;
    Fw f = {{0}, 0};
    uint64_t start;
    int i, ok = 1;

    set(&f, UCSR0B, 0x08);
    delay(&f, 10);
    set(&f, UDR0, 'A');
    set(&f, UDR0, 'B');
    halt(&f);
    rig_new(&r, "TESTAVR", NULL);
    load(&r, &f);
    if (rig_start(&r) != 0) return;
    if (!wait_for(&r, "PD1", CIRCUIT_1, 3 * MS) || !wait_for(&r, "PD1", CIRCUIT_0, MS)) {
        check(0, "usart: a start bit");
        circuit_destroy(r.c);
        return;
    }
    start = circuit_time(r.c);            /* within 1 us of the fall */
    for (i = 0; i < 20; i++) {
        rig_to(&r, start + (uint64_t)i * 16 * US + 7 * US);
        ok &= (level(&r, "PD1") == CIRCUIT_1) == WANT[i];
    }
    check(ok, "usart: two bytes written back to back go out as two 8N1 frames, 16 us a bit");
    circuit_destroy(r.c);
}

static void usart_glitch(void) {
    Rig r;
    Fw f = {{0}, 0};
    size_t sw;
    int glitch;

    set(&f, PORTD, 0x01);                 /* RXD's pull-up */
    set(&f, UCSR0B, 0x10);                /* RXEN */
    set(&f, DDRB, 0x01);
    wait_bit(&f, UCSR0A, 7);              /* RXC */
    set(&f, PORTB, 0x01);
    halt(&f);
    rig_new(&r, "TESTAVR", NULL);
    load(&r, &f);
    sw = rig_two_pin(&r, "AS11AH", rig_net(&r, "PD0"), r.n_gnd);
    if (rig_start(&r) != 0) return;
    rig_to(&r, 3 * MS);
    circuit_set_switch(r.c, sw, 1, 1);    /* 3 us low: high again at the start bit's middle */
    circuit_advance(r.c, 3 * US);
    circuit_set_switch(r.c, sw, 1, 0);
    circuit_advance(r.c, MS);
    glitch = level(&r, "PB0") == CIRCUIT_0;
    circuit_set_switch(r.c, sw, 1, 1);    /* a frame of 0x00: 9 bits low, then the stop bit */
    circuit_advance(r.c, 144 * US);
    circuit_set_switch(r.c, sw, 1, 0);
    circuit_advance(r.c, MS);
    check(glitch && level(&r, "PB0") == CIRCUIT_1,
          "usart: a 3 us low on RXD is no start bit; a real frame is received");
    circuit_destroy(r.c);
}

/* ---- Reset and clock -------------------------------------------------------------- */

/* Boot 1 (no WDRF): PB0 high, the watchdog on (16 ms, system reset).
   Boot 2 (WDRF): PB1 high, WDRF cleared and the watchdog off. */
static void watchdog_firmware(Fw *f) {
    size_t jump;

    set(f, DDRB, 0x03);
    lds(f, 18, MCUSR);
    op(f, (uint16_t)(0xFE00 | (18 << 4) | 3));   /* SBRS r18, 3 (WDRF) */
    jump = f->n;
    op(f, 0xC000);                               /* RJMP boot 1, patched below */
    set(f, PORTB, 0x02);                         /* boot 2 */
    set(f, MCUSR, 0x00);
    op(f, ldi(16, 0x18));
    op(f, ldi(17, 0x00));
    sts(f, WDTCSR, 16);
    sts(f, WDTCSR, 17);
    halt(f);
    f->w[jump] = (uint16_t)(0xC000 | ((f->n - jump - 1) & 0x0FFF));
    set(f, PORTB, 0x01);                         /* boot 1 */
    op(f, ldi(16, 0x18));                        /* WDCE | WDE, then WDE */
    op(f, ldi(17, 0x08));
    sts(f, WDTCSR, 16);
    sts(f, WDTCSR, 17);
    halt(f);
}

static void watchdog_reset(int crystal) {
    Rig r;
    Fw f = {{0}, 0};
    int first;

    watchdog_firmware(&f);
    rig_new(&r, "TESTAVR", NULL);
    load(&r, &f);
    if (crystal) rig_two_pin(&r, "GENERIC-CRYSTAL-16M", r.net[P_PB6], r.net[P_PB7]);
    if (rig_start(&r) != 0) return;
    rig_to(&r, 5 * MS);
    first = level(&r, "PB0") == CIRCUIT_1 && level(&r, "PB1") == CIRCUIT_0;
    rig_to(&r, 40 * MS);
    check(first && level(&r, "PB0") == CIRCUIT_0 && level(&r, "PB1") == CIRCUIT_1,
          crystal ? "reset: a watchdog reset on a 16 MHz crystal starts over with WDRF"
                  : "reset: a watchdog reset starts the program over with WDRF");
    circuit_destroy(r.c);
}

static void reset_held(void) {
    Rig r;
    size_t sw;
    CpuState state;

    rig_new(&r, "TESTAVR", "blink");
    rig_two_pin(&r, "GENERIC-CRYSTAL-16M", r.net[P_PB6], r.net[P_PB7]);
    sw = rig_two_pin(&r, "AS11AH", r.net[P_PC6], r.n_gnd);
    if (rig_start(&r) != 0) return;
    circuit_set_switch(r.c, sw, 1, 1);    /* low before the startup time ends */
    rig_to(&r, 5 * MS);
    check(circuit_net_value(r.c, r.net[P_PB5]) == CIRCUIT_Z &&
              circuit_cpu_state(r.c, r.avr, &state) == 0 && state.status == CPU_RESET,
          "reset: RESET low from power-up: the chip stays in reset (crystal running)");
    circuit_destroy(r.c);
}

static void clkpr_crystal(void) {
    Rig r;
    Fw f = {{0}, 0};

    op(&f, ldi(16, 0x80));
    op(&f, ldi(17, 0x01));
    sts(&f, CLKPR, 16);                   /* CLKPCE, then CLKPS = 0001: /2 */
    sts(&f, CLKPR, 17);
    set(&f, DDRB, 0x01);
    op(&f, 0x9A18);                       /* SBI PINB, 0: toggle PB0 (2 cycles) */
    op(&f, 0xCFFE);                       /* RJMP .-2 (2 cycles) */
    rig_new(&r, "TESTAVR", NULL);
    load(&r, &f);
    rig_two_pin(&r, "GENERIC-CRYSTAL-16M", r.net[P_PB6], r.net[P_PB7]);
    if (rig_start(&r) != 0) return;
    rig_to(&r, 2 * MS);
    check(toggle_period(&r, rig_net(&r, "PB0"), 62500, MS, 125000) == 500000,
          "clock: CLKPR /2 on a 16 MHz crystal: 8 MHz, PB0 toggles every 4 cycles (500 ns)");
    circuit_destroy(r.c);
}

/* ---- Pinouts without PC6 or PB5, and the flash ------------------------------------------ */

#define P28_PART(NAME) \
    "'parts':[{'name':'" NAME "','manufacturer':'Generic','package':'DIP','pinout':'p28'," \
    "'footprint':'PDIP-28-MC-28P3'}]"

static void missing_pins(void) {
    const Part *no_reset = s68_loads("mcu", "atmega328p", "{'pinouts.p28.0.name':'RST'," P28_PART("NORST") "}");
    const Part *no_sck = s68_loads("mcu", "atmega328p", "{'pinouts.p28.18.name':'SCKX'," P28_PART("NOSCK") "}");
    Rig r;
    Fw f = {{0}, 0};

    if (no_reset == NULL || no_sck == NULL) {
        check(0, "pins: the fixtures load");
        return;
    }
    set(&f, DDRB, 0x01);
    set(&f, PORTB, 0x01);
    halt(&f);
    rig_new(&r, no_reset->name, NULL);
    load(&r, &f);
    if (rig_start(&r) != 0) return;
    rig_to(&r, 3 * MS);
    check(level(&r, "PB0") == CIRCUIT_1, "pins: with no reset pin in the pinout the chip runs");
    circuit_destroy(r.c);

    f.n = 0;
    set(&f, DDRB, 0x09);                  /* PB0, MOSI */
    set(&f, SPCR, 0x50);
    set(&f, SPDR, 0x5A);
    wait_bit(&f, SPSR, 7);
    set(&f, PORTB, 0x01);
    halt(&f);
    rig_new(&r, no_sck->name, NULL);
    load(&r, &f);
    if (rig_start(&r) != 0) return;
    rig_to(&r, 3 * MS);
    check(level(&r, "PB0") == CIRCUIT_1, "pins: with no SCK pin the SPI byte still completes");
    circuit_destroy(r.c);
}

static void flash_read(void) {
    Rig r;
    uint32_t values[4], known[4];

    rig_new(&r, "TESTAVR", NULL);
    check(circuit_memory_read(r.c, r.avr, 32768, 4, values, known) == 0 &&
              circuit_memory_read(r.c, r.avr, 32766, 4, values, known) == 2 && values[1] == 0xff,
          "flash: a read at the end gives nothing; one across it stops at the end");
    circuit_destroy(r.c);
}

int main(void) {
    library = s68_library();
    if (load_mcu_fixture(library) != 0) {
        printf("FAIL: cannot load the mcu fixture\n");
        return 1;
    }
    twi_abandon();
    spi_abort();
    spi_slave();
    usart_tx_disabled();
    usart_two_frames();
    usart_glitch();
    watchdog_reset(0);
    watchdog_reset(1);
    reset_held();
    clkpr_crystal();
    missing_pins();
    flash_read();
    s68_free();
    if (failures > 0) {
        printf("mcu_cov_test: %d failures\n", failures);
        return 1;
    }
    printf("mcu_cov_test: all passed\n");
    return 0;
}
