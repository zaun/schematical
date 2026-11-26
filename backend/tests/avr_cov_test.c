/*
 * avr_cov_test (plan-68-S): the AVR core and peripherals (src/models/avr_*.c)
 * through their C API, on bare chips (avr_chip_create, no mcu around them):
 * what avr_core_test, avr_periph_test and the mcu programs do not reach.
 *
 *   events   the clock-event pool of avr_cpu.c: a reset inside an event, a
 *            full pool, an event that clears itself (avr8js's semantics, which
 *            the port keeps; nothing may leak or be freed twice)
 *   pc22     a flash above 128 KB: 3-byte return addresses (AVR instruction
 *            set manual, CALL/RET/interrupts), ELPM Z+ carrying into RAMPZ
 *   wrap     RJMP below address 0 wraps; a 2-word instruction at the end
 *   timer0   FOC0A with COM0A = 00 leaves the pin a port pin (ATmega328P
 *            15.7.2 table 15-2, 15.9.2); phase-correct PWM with TOP 0; OCR0A
 *            loaded when the CPU writes TCNT0 = 0 (BOTTOM) in fast PWM
 *   timer1   the ATtiny85's Timer1 (Atmel 2586 chapter 12): FOC1B, /OC1B,
 *            COM1A = 00 in PWM, channel B's compare output beside PWM A,
 *            and the GTCCR/TIFR hooks it shares with Timer0
 *   twi      no handler: avr8js's do-nothing bus (start, an address nobody
 *            answers, stop); TWPS (datasheet 22.5.2); a reset with a request
 *            still queued
 *   usi      the strobe clock, an external clock counting both edges, the
 *            buffer when USITC overflows the counter (Atmel 2586 15.5)
 *   gpio     EIFR: a 1 written clears INTF0
 *   adc      REFS2 (2.56 V) on the ATtiny85, an 8-reference config, MUX5,
 *            the on_read listener
 *   spi/usart  no listener: avr8js's 0 from nobody; the rx-complete callback
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "avr_chip.h"
#include "cJSON.h"
#include "memory_image.h"
#include "read_file.h"

static int failures;

static void check(int ok, const char *what) {
    printf("%s %s\n", ok ? "ok" : "FAIL:", what);
    if (!ok) {
        failures++;
    }
}

/* A chip of the layout with NOPs in all of its flash. */
static AvrChip *nop_chip(const char *layout, uint32_t flash) {
    AvrChip *chip = avr_chip_create(avr_layout_find(layout), flash, 512, 512, 8000000, 0);

    memset(chip->cpu->flash, 0, flash);
    return chip;
}

static void steps(AvrChip *chip, int n) {
    while (n-- > 0) {
        avr_chip_step(chip);
    }
}

static void put(AvrChip *chip, uint16_t addr, uint8_t value) {
    avr_write_data(chip->cpu, addr, value, 0xff);
}

static uint8_t get(AvrChip *chip, uint16_t addr) {
    return avr_read_data(chip->cpu, addr);
}

/* ---- Clock events ------------------------------------------------------------- */

typedef struct {
    AvrCpu *cpu;
    int calls;
    int cleared;   /* avr_clear_clock_event's answer from inside the callback */
} Counter;

static void count_fn(void *ctx) {
    ((Counter *)ctx)->calls++;
}

static Counter late;

static void reset_fn(void *ctx) {
    Counter *c = ctx;

    c->calls++;
    avr_cpu_reset(c->cpu);
    avr_add_clock_event(c->cpu, count_fn, &late, 1);
}

static void self_clear_fn(void *ctx) {
    Counter *c = ctx;

    c->calls++;
    c->cleared = avr_clear_clock_event(c->cpu, self_clear_fn, ctx);
}

static void tick_to(AvrCpu *cpu, uint64_t cycles) {
    while (cpu->cycles < cycles) {
        cpu->cycles++;
        avr_cpu_tick(cpu);
    }
}

static void event_cases(void) {
    AvrCpu *cpu = avr_cpu_create(1024, 256);
    Counter r = {0}, after = {0}, many[12], s = {0};
    int i, all = 1;

    /* A reset inside an event: the events added after it are dropped, the
       rest of the old list comes back (avr8js's tick). */
    r.cpu = cpu;
    memset(&late, 0, sizeof late);
    avr_add_clock_event(cpu, reset_fn, &r, 1);
    avr_add_clock_event(cpu, count_fn, &after, 5);
    tick_to(cpu, 10);
    check(r.calls == 1 && late.calls == 0 && after.calls == 1,
          "events: a reset in an event drops the events added after it, keeps the old list");

    /* Twelve events cleared before they run: none runs; the pool holds 10. */
    memset(many, 0, sizeof many);
    for (i = 0; i < 12; i++) avr_add_clock_event(cpu, count_fn, &many[i], 3);
    for (i = 0; i < 12; i++) all &= avr_clear_clock_event(cpu, count_fn, &many[i]);
    tick_to(cpu, 20);
    for (i = 0; i < 12; i++) all &= many[i].calls == 0;
    check(all && cpu->event_pool_count == AVR_EVENT_POOL,
          "events: 12 cleared events never run; the pool keeps 10 nodes");

    /* The pool is full: an event clearing itself finds itself (1) and runs once. */
    s.cpu = cpu;
    avr_add_clock_event(cpu, self_clear_fn, &s, 1);
    tick_to(cpu, 25);
    check(s.calls == 1 && s.cleared == 1, "events: a self-clearing event with a full pool runs once");
    avr_cpu_destroy(cpu);

    /* Nine nodes pooled: the self-cleared node fills the pool, and the tick
       finds it already there. */
    cpu = avr_cpu_create(1024, 256);
    memset(many, 0, sizeof many);
    for (i = 0; i < 9; i++) avr_add_clock_event(cpu, count_fn, &many[i], i + 1);
    tick_to(cpu, 10);
    memset(&s, 0, sizeof s);
    s.cpu = cpu;
    avr_add_clock_event(cpu, count_fn, &many[10], 1);   /* takes a pooled node */
    tick_to(cpu, 12);                                   /* back: 9 pooled */
    avr_add_clock_event(cpu, self_clear_fn, &s, 1);     /* takes one: 8 */
    avr_add_clock_event(cpu, count_fn, &many[11], 50);  /* 7 */
    avr_clear_clock_event(cpu, count_fn, &many[11]);    /* 8 */
    avr_add_clock_event(cpu, count_fn, &many[11], 50);  /* 7 */
    avr_clear_clock_event(cpu, count_fn, &many[11]);    /* 8 */
    {
        /* One more pooled node: 9 when the self-clear runs. */
        Counter extra = {0};

        avr_add_clock_event(cpu, count_fn, &extra, 100);
        avr_clear_clock_event(cpu, count_fn, &extra);
    }
    tick_to(cpu, 14);
    check(s.calls == 1 && s.cleared == 1 && cpu->event_pool_count == AVR_EVENT_POOL,
          "events: a self-cleared node already in the pool is not pooled twice");
    avr_cpu_destroy(cpu);

    /* An empty pool: the self-cleared node goes into it twice (avr8js); the
       next event takes one copy while the other stays pooled. */
    cpu = avr_cpu_create(1024, 256);
    memset(&s, 0, sizeof s);
    memset(&after, 0, sizeof after);
    s.cpu = cpu;
    avr_add_clock_event(cpu, self_clear_fn, &s, 1);
    tick_to(cpu, 3);
    check(s.calls == 1 && s.cleared == 1 && cpu->event_pool_count == 2 &&
              cpu->event_pool[0] == cpu->event_pool[1],
          "events: a self-cleared node with room in the pool is pooled twice");
    avr_cpu_destroy(cpu);   /* frees it once */

    cpu = avr_cpu_create(1024, 256);
    memset(&s, 0, sizeof s);
    s.cpu = cpu;
    avr_add_clock_event(cpu, self_clear_fn, &s, 1);
    tick_to(cpu, 3);
    avr_add_clock_event(cpu, count_fn, &after, 100);
    check(cpu->event_pool_count == 1 && cpu->next_event == cpu->event_pool[0],
          "events: the node queued again is still in the pool");
    avr_cpu_destroy(cpu);   /* queued and pooled: freed once */
}

/* ---- Program counter ---------------------------------------------------------- */

static void flash_word(AvrCpu *cpu, uint32_t word, uint16_t value) {
    cpu->flash[word * 2] = (uint8_t)value;
    cpu->flash[word * 2 + 1] = (uint8_t)(value >> 8);
}

static void pc22_cases(void) {
    AvrCpu *cpu = avr_cpu_create(0x40000, 2048);   /* 256 KB: a 22-bit pc */
    uint16_t sp;

    memset(cpu->flash, 0, 0x40000);
    /* An interrupt pushes 3 bytes: low at SP, then middle, then high. */
    cpu->pc = 0x12345;
    sp = avr_get16(cpu, AVR_SP);
    avr_interrupt(cpu, 0x10);
    check(cpu->pc == 0x10 && avr_get16(cpu, AVR_SP) == sp - 3 && cpu->data[sp] == 0x45 &&
              cpu->data[sp - 1] == 0x23 && cpu->data[sp - 2] == 0x01,
          "pc22: an interrupt pushes the 3-byte pc");

    /* CALL 0x18000 at 0x100 pushes 0x000102 (3 bytes); RET there comes back. */
    avr_set16(cpu, AVR_SP, sp);
    cpu->pc = 0x100;
    flash_word(cpu, 0x100, 0x940F);   /* CALL k, k21:16 = 0x01 */
    flash_word(cpu, 0x101, 0x8000);
    flash_word(cpu, 0x18000, 0x9508); /* RET */
    avr_instruction(cpu);
    check(cpu->pc == 0x18000 && avr_get16(cpu, AVR_SP) == sp - 3 && cpu->data[sp] == 0x02 &&
              cpu->data[sp - 1] == 0x01 && cpu->data[sp - 2] == 0x00,
          "pc22: CALL pushes the 3-byte return address");
    avr_instruction(cpu);
    check(cpu->pc == 0x102 && avr_get16(cpu, AVR_SP) == sp, "pc22: RET pops 3 bytes");

    /* ELPM r0, Z+ at RAMPZ:Z = 01:FFFF reads byte 0x1FFFF; Z+ carries into RAMPZ. */
    cpu->flash[0x1FFFF] = 0xA7;
    cpu->data[0x5B] = 0x01;
    cpu->data[30] = 0xFF;
    cpu->data[31] = 0xFF;
    flash_word(cpu, 0x102, 0x9007);
    avr_instruction(cpu);
    check(cpu->data[0] == 0xA7 && cpu->data[30] == 0 && cpu->data[31] == 0 && cpu->data[0x5B] == 2,
          "pc22: ELPM Z+ from 01:FFFF reads the byte and goes on at 02:0000");
    avr_cpu_destroy(cpu);

    cpu = avr_cpu_create(0x8000, 2048);
    memset(cpu->flash, 0, 0x8000);
    /* RJMP .-2 at address 0: the pc wraps to the last word. */
    flash_word(cpu, 0, 0xCFFE);
    cpu->pc = 0;
    avr_instruction(cpu);
    check(cpu->pc == 0x3FFF, "wrap: RJMP below address 0 lands on the last word");
    /* CALL in the last word: its address word is past the end (word 0, a 0). */
    flash_word(cpu, 0x3FFF, 0x940E);
    avr_instruction(cpu);
    check(cpu->pc == 0, "wrap: a CALL in the last word reads its address as 0");
    avr_cpu_destroy(cpu);
}

/* ---- Chip API ------------------------------------------------------------------- */

static int after_resets;

static void count_reset(void *ctx) {
    (void)ctx;
    after_resets++;
}

/* tests/avr/watchdog.json's program into a chip's flash. */
static int load_watchdog(AvrChip *chip) {
    char path[1024];
    char *text;
    cJSON *json;
    const char *program;
    uint32_t flash = chip->cpu->flash_bytes;
    uint32_t *image = malloc(flash * sizeof *image);
    char *error = NULL;
    uint32_t i;
    int result = -1;

    snprintf(path, sizeof path, "%s/watchdog.json", AVR_DIR);
    text = read_file(path);
    json = text != NULL ? cJSON_Parse(text) : NULL;
    program = cJSON_GetStringValue(cJSON_GetObjectItemCaseSensitive(json, "program"));
    if (program != NULL && memory_image_parse((const unsigned char *)program, strlen(program),
                                              MEMORY_IMAGE_INTEL_HEX, flash, 8, image, &error) == 0) {
        for (i = 0; i < flash; i++) chip->cpu->flash[i] = (uint8_t)image[i];
        result = 0;
    }
    free(error);
    free(image);
    cJSON_Delete(json);
    free(text);
    return result;
}

/* INT0 on a falling edge sets INTF0; writing 1 to it clears it (ATmega328P 13.2.3). */
static void eifr_case(void) {
    AvrChip *chip = nop_chip("atmega328", 32768);
    AvrPort *d = avr_chip_port(chip, 'D');
    int set;

    avr_port_set_pin(d, 2, 1);
    put(chip, 0x69, 0x02);                            /* EICRA: ISC01, falling edge */
    put(chip, 0x3D, 0x01);                            /* EIMSK: INT0 */
    avr_port_set_pin(d, 2, 0);
    set = (get(chip, 0x3C) & 0x01) != 0;
    put(chip, 0x3C, 0x01);
    check(set && (get(chip, 0x3C) & 0x01) == 0, "gpio: INTF0 set by a falling INT0, cleared by a 1");
    avr_chip_destroy(chip);
}

static void chip_cases(void) {
    AvrChip *chip = avr_chip_create(avr_layout_find("atmega328"), 32768, 2048, 1024, 16000000, 0);
    long n;

    check(avr_chip_port(chip, 'Z') == NULL && avr_chip_port(chip, 'B') == &chip->ports[0],
          "chip: no port Z; port B is the first");
    check(avr_cpu_port_at(chip->cpu, 0) == NULL && avr_cpu_port_at(chip->cpu, 0x99) == NULL,
          "chip: no port at address 0 or at a non-port register");
    chip->after_reset.fn = count_reset;
    if (load_watchdog(chip) != 0) {
        check(0, "chip: watchdog program");
    }
    for (n = 0; n < 200000 && chip->cpu->data[21] < 2; n++) avr_chip_step(chip);
    check(after_resets >= 1 && chip->cpu->data[21] == 2,
          "chip: a watchdog reset calls the after_reset hook");
    avr_chip_destroy(chip);
}

/* ---- Timer0 (ATmega328P) and Timer1 (ATtiny85) ---------------------------------- */

enum {
    M_DDRD = 0x2A, M_PORTD = 0x2B, M_TIFR0 = 0x35, M_TCCR0A = 0x44, M_TCCR0B = 0x45,
    M_TCNT0 = 0x46, M_OCR0A = 0x47
};

static void timer0_cases(void) {
    AvrChip *chip = nop_chip("atmega328", 32768);
    AvrPort *d = avr_chip_port(chip, 'D');

    put(chip, M_DDRD, 0x40);                        /* PD6 (OC0A) an output, low */
    put(chip, M_TCCR0B, 0x80);                      /* FOC0A, COM0A = 00 */
    put(chip, M_PORTD, 0x40);
    check(avr_port_pin_state(d, 6) == AVR_PIN_HIGH,
          "timer0: FOC0A with COM0A = 00 leaves PD6 a port pin (PORTD6 1: high)");
    put(chip, M_PORTD, 0x00);
    put(chip, M_TCCR0A, 0x40);                      /* COM0A = 01: toggle */
    put(chip, M_TCCR0B, 0x80);
    check(avr_port_pin_state(d, 6) == AVR_PIN_HIGH, "timer0: FOC0A with COM0A = 01 toggles OC0A");
    avr_chip_destroy(chip);

    /* Phase-correct PWM, TOP = OCR0A = 0: the counter stays at 0. */
    chip = nop_chip("atmega328", 32768);
    put(chip, M_OCR0A, 0);
    put(chip, M_TCCR0A, 0x01);
    put(chip, M_TCCR0B, 0x09);                      /* WGM02, clk/1 */
    steps(chip, 50);
    check(get(chip, M_TCNT0) == 0, "timer0: phase-correct PWM with TOP 0 stays at 0");
    avr_chip_destroy(chip);

    /* Fast PWM: OCR0A is buffered until BOTTOM; a CPU write of TCNT0 = 0 is BOTTOM. */
    chip = nop_chip("atmega328", 32768);
    put(chip, M_TCCR0A, 0x03);                      /* fast PWM, TOP 0xFF */
    put(chip, M_OCR0A, 0x80);
    put(chip, M_TCCR0B, 0x01);
    put(chip, M_TCNT0, 0x00);
    steps(chip, 2);
    put(chip, M_TIFR0, 0x07);
    steps(chip, 0x40);
    check((get(chip, M_TIFR0) & 0x02) == 0, "timer0: OCR0A = 0x80 loaded at the written BOTTOM");
    steps(chip, 0x60);
    check((get(chip, M_TIFR0) & 0x02) != 0, "timer0: OCF0A once TCNT0 passes 0x80");
    avr_chip_destroy(chip);
}

enum {
    T_DDRB = 0x37, T_PORTB = 0x38, T_GTCCR = 0x4C, T_OCR1C = 0x4D, T_OCR1A = 0x4E, T_OCR1B = 0x4B,
    T_TCCR1 = 0x50, T_TIFR = 0x58
};

static int pb(AvrChip *chip, int bit) {
    AvrPinState s = avr_port_pin_state(&chip->ports[0], bit);

    return s == AVR_PIN_HIGH ? 1 : s == AVR_PIN_LOW ? 0 : -1;
}

static uint8_t gtccr_seen;

static int gtccr_recorder(void *ctx, uint8_t value, uint8_t old_value, uint16_t addr, uint8_t mask) {
    (void)ctx;
    (void)old_value;
    (void)addr;
    (void)mask;
    gtccr_seen = value;
    return 1;
}

static void timer1_cases(void) {
    AvrChip *chip = nop_chip("attiny85", 8192);
    int changes = 0, last, i, differ = 1;

    /* FOC1B, non-PWM, COM1B = 01: OC1B (PB4) toggles. */
    put(chip, T_DDRB, 0x10);
    put(chip, T_GTCCR, 0x10);
    put(chip, T_GTCCR, 0x18);
    check(pb(chip, 4) == 1, "timer1: FOC1B with COM1B = 01 toggles OC1B");
    avr_chip_destroy(chip);

    /* PWM1B with COM1B = 01: OC1B and /OC1B (PB3) both connected, complementary. */
    chip = nop_chip("attiny85", 8192);
    put(chip, T_DDRB, 0x18);
    put(chip, T_OCR1C, 0xFF);
    put(chip, T_OCR1B, 0x40);
    put(chip, T_GTCCR, 0x50);
    put(chip, T_TCCR1, 0x01);
    steps(chip, 300);                                 /* past the first TOP */
    for (i = 0; i < 40; i++) {
        steps(chip, 37);
        differ &= pb(chip, 3) == !pb(chip, 4);
    }
    check(differ, "timer1: PWM1B, COM1B = 01: /OC1B is the complement of OC1B");
    avr_chip_destroy(chip);

    /* PWM1A with COM1A = 00: OC1A (PB1) stays the port's; channel B, not in
       PWM, toggles OC1B (PB4) at each OCR1B match. */
    chip = nop_chip("attiny85", 8192);
    put(chip, T_DDRB, 0x12);
    put(chip, T_PORTB, 0x02);
    put(chip, T_OCR1C, 0x20);
    put(chip, T_OCR1A, 0x10);
    put(chip, T_OCR1B, 0x08);
    put(chip, T_GTCCR, 0x10);
    put(chip, T_TCCR1, 0x41);
    last = pb(chip, 4);
    for (i = 0; i < 100; i++) {
        steps(chip, 5);
        if (pb(chip, 4) != last) changes++;
        last = pb(chip, 4);
        if (pb(chip, 1) != 1) changes = -1000;
    }
    check(changes >= 2, "timer1: COM1A = 00 leaves OC1A to the port; OC1B toggles beside PWM A");
    avr_chip_destroy(chip);

    /* On a bare CPU: an earlier GTCCR hook gets the write without FOC1A,
       FOC1B and PSR1 (they read 0); with no earlier TIFR hook, writing 1 to
       OCF1A clears it. */
    {
        AvrCpu *cpu = avr_cpu_create(8192, 512);
        AvrTinyTimer1 t;

        cpu->write_hooks[T_GTCCR].fn = gtccr_recorder;
        avr_tiny_timer1_init(&t, cpu, &avr_layout_find("attiny85")->tiny_timer1);
        avr_write_data(cpu, T_GTCCR, 0x0F, 0xff);
        check(gtccr_seen == 0x01, "timer1: GTCCR's earlier hook sees PSR0 only");
        avr_set_interrupt_flag(cpu, &t.OCFA);
        avr_write_data(cpu, T_TIFR, 0x40, 0xff);
        check((cpu->data[T_TIFR] & 0x40) == 0, "timer1: TIFR written alone: OCF1A cleared");
        avr_cpu_destroy(cpu);
    }
}

/* ---- TWI, SPI, USART -------------------------------------------------------------- */

enum {
    TWBR = 0xB8, TWSR = 0xB9, TWDR = 0xBB, TWCR = 0xBC, SPCR = 0x4C, SPSR = 0x4D, SPDR = 0x4E,
    UCSR0B = 0xC1, UDR0 = 0xC6
};

static AvrChip *reset_chip;

static void reset_in_event(void *ctx) {
    (void)ctx;
    avr_chip_reset(reset_chip);
}

static void twi_cases(void) {
    AvrChip *chip = nop_chip("atmega328", 32768);
    AvrTwi *twi = &chip->twis[0];
    static const int PRESCALE[4] = {1, 4, 16, 64};
    int i, ok = 1;

    put(chip, TWBR, 10);
    for (i = 0; i < 4; i++) {
        put(chip, TWSR, (uint8_t)i);
        ok &= avr_twi_prescaler(twi) == PRESCALE[i] &&
              avr_twi_bit_cycles(twi) == 16u + 2u * 10u * (unsigned)PRESCALE[i];
    }
    check(ok, "twi: TWPS 0-3 divide by 1, 4, 16, 64: 16 + 2 x TWBR x prescaler cycles a bit");
    /* avr8js's TWSR stores its status bits too: a reset puts F8 back. */
    avr_chip_reset(chip);
    /* No handler: avr8js's bus where nobody answers. */
    put(chip, TWCR, 0xA4);                            /* TWINT, TWSTA, TWEN */
    steps(chip, 3);
    ok = (get(chip, TWSR) & 0xF8) == 0x08 && (get(chip, TWCR) & 0x80);
    put(chip, TWDR, 0xA0);
    put(chip, TWCR, 0x84);                            /* SLA+W */
    steps(chip, 3);
    ok &= (get(chip, TWSR) & 0xF8) == 0x20;
    put(chip, TWCR, 0x94);                            /* TWSTO */
    steps(chip, 3);
    ok &= (get(chip, TWSR) & 0xF8) == 0xF8 && !(get(chip, TWCR) & 0x10);
    check(ok, "twi: no handler: START 08, SLA+W not acknowledged 20, STOP back to F8");
    /* avr8js's register: a status written into TWSR (exact mode) is what
       the next request sees; nobody acknowledges the data, 0xFF comes in. */
    chip->cpu->exact = 1;
    put(chip, TWSR, 0x18);
    put(chip, TWCR, 0x84);
    steps(chip, 3);
    ok = (get(chip, TWSR) & 0xF8) == 0x30;
    put(chip, TWSR, 0x40);
    put(chip, TWCR, 0xC4);
    steps(chip, 3);
    ok &= (get(chip, TWSR) & 0xF8) == 0x50 && get(chip, TWDR) == 0xFF;
    check(ok, "twi: exact, no handler: data not acknowledged 30, a read gives FF (50)");
    chip->cpu->exact = 0;
    /* A chip reset: TWCR's initial value 0 (TWINT clear), TWSR F8. */
    avr_chip_reset(chip);
    check(get(chip, TWCR) == 0x00 && get(chip, TWSR) == 0xF8, "twi: after a reset TWCR 00, TWSR F8");
    /* A reset while a request waits: the request is gone (TWINT stays 0). */
    reset_chip = chip;
    put(chip, TWCR, 0xA4);
    avr_add_clock_event(chip->cpu, reset_in_event, NULL, 0);   /* runs first, same cycle */
    steps(chip, 4);
    check(!(get(chip, TWCR) & 0x80) && (get(chip, TWSR) & 0xF8) == 0xF8,
          "twi: a reset before the request runs drops it");
    avr_chip_destroy(chip);
}

static int rx_completes;

static void rx_complete(void *ctx) {
    (void)ctx;
    rx_completes++;
}

static void spi_usart_cases(void) {
    AvrChip *chip = nop_chip("atmega328", 32768);
    AvrUsart *usart = &chip->usarts[0];

    /* No byte listener: the transfer ends with 0 after its cycles. */
    put(chip, SPCR, 0x50);                            /* SPE, MSTR, clk/4 */
    put(chip, SPDR, 0x5A);
    steps(chip, 40);
    check((get(chip, SPSR) & 0x80) && get(chip, SPDR) == 0x00,
          "spi: nobody listening: SPIF, and 0 came in");

    usart->on_rx_complete.fn = rx_complete;
    put(chip, UCSR0B, 0x10);                          /* RXEN */
    check(avr_usart_write_byte(usart, 'x', 1) == 1 && rx_completes == 1 && get(chip, UDR0) == 'x',
          "usart: a byte received calls on_rx_complete; UDR reads it");
    avr_chip_destroy(chip);
}

/* ---- USI (ATtiny85) ---------------------------------------------------------------- */

enum { USICR = 0x2D, USISR = 0x2E, USIDR = 0x2F, USIBR = 0x30 };

static void usi_cases(void) {
    AvrChip *chip = nop_chip("attiny85", 8192);
    AvrPort *b = &chip->ports[0];

    /* USICS = 00, USICLK written 1: one shift (DI, PB0, high) and one count. */
    avr_port_set_pin(b, 0, 1);
    put(chip, USIDR, 0x40);
    put(chip, USICR, 0x12);                           /* three-wire, USICLK */
    check(get(chip, USIDR) == 0x81 && (get(chip, USISR) & 0x0F) == 1,
          "usi: the USICLK strobe shifts DI in and counts once");
    avr_chip_destroy(chip);

    /* USICS = 10, USICLK 0: shift on USCK's rising edge, count both edges. */
    chip = nop_chip("attiny85", 8192);
    b = &chip->ports[0];
    avr_port_set_pin(b, 0, 1);
    put(chip, USICR, 0x18);
    avr_port_set_pin(b, 2, 1);
    avr_port_set_pin(b, 2, 0);
    avr_port_set_pin(b, 2, 1);
    avr_port_set_pin(b, 2, 0);
    check((get(chip, USISR) & 0x0F) == 4 && (get(chip, USIDR) & 0x03) == 0x03,
          "usi: external clock: 2 shifts on the rises, 4 counts");
    avr_chip_destroy(chip);

    /* USITC with USICS1 and USICLK at count 15: the counter overflows now,
       the shift comes with the pin's edge, and the buffer takes the byte with it. */
    chip = nop_chip("attiny85", 8192);
    b = &chip->ports[0];
    avr_port_set_pin(b, 0, 1);
    put(chip, T_DDRB, 0x04);
    put(chip, USISR, 0x0F);
    put(chip, USICR, 0x1B);                           /* USICS1, USICLK, USITC */
    avr_port_set_pin(b, 2, 1);                        /* USCK follows its PORT bit */
    check(get(chip, USIDR) == 0x01 && get(chip, USIBR) == 0x01 && (get(chip, USISR) & 0x40),
          "usi: an overflow from USITC: the buffer gets the byte with the edge's bit");
    avr_chip_destroy(chip);

    /* No port at the USI's address (a bare CPU): no hooks. */
    {
        AvrCpu *cpu = avr_cpu_create(8192, 512);
        AvrUsi usi;

        avr_usi_init(&usi, cpu, &avr_layout_find("attiny85")->usi);
        check(usi.port == NULL && cpu->write_hooks[USICR].fn == NULL,
              "usi: without its port the USI hooks nothing");
        avr_cpu_destroy(cpu);
    }
}

/* ---- ADC ---------------------------------------------------------------------------- */

enum {
    M_ADCL = 0x78, M_ADCH = 0x79, M_ADCSRA = 0x7A, M_ADCSRB = 0x7B, M_ADMUX = 0x7C,
    T_ADCL = 0x24, T_ADCH = 0x25, T_ADCSRA = 0x26, T_ADMUX = 0x27
};

static int adc_result(AvrChip *chip, uint16_t adcl, uint16_t adch) {
    return get(chip, adcl) | (get(chip, adch) << 8);
}

static const AvrAdcInput *seen_input;
static AvrAdc *listened;

static void on_read(void *ctx, const AvrAdcInput *input) {
    (void)ctx;
    seen_input = input;
    avr_adc_complete_read(listened, 0x155);
}

static void adc_cases(void) {
    AvrChip *chip = nop_chip("attiny85", 8192);

    /* REFS2:0 = 110 with ADC1 (PB2) at 1.28 V: 2.56 V reference, 512. */
    chip->adc.channel_volts[1] = 1.28;
    put(chip, T_ADMUX, 0x91);
    put(chip, T_ADCSRA, 0xC0);
    steps(chip, 400);
    check(adc_result(chip, T_ADCL, T_ADCH) == 512, "adc: ATtiny85 REFS2:0 = 110 is 2.56 V");
    avr_chip_destroy(chip);

    /* ATmega328P: ADCSRB bit 3 is reserved (no MUX5): channel 0 at 2.5 V reads 512. */
    chip = nop_chip("atmega328", 32768);
    chip->adc.channel_volts[0] = 2.5;
    put(chip, M_ADCSRB, 0x08);
    put(chip, M_ADMUX, 0x40);                         /* AVCC (5 V), ADC0 */
    put(chip, M_ADCSRA, 0xC0);
    steps(chip, 400);
    check(adc_result(chip, M_ADCL, M_ADCH) == 512, "adc: ADCSRB bit 3 does not change the channel");

    /* The listener: it gets the input and gives the result. */
    listened = &chip->adc;
    chip->adc.on_read.fn = on_read;
    put(chip, M_ADMUX, 0x41);
    put(chip, M_ADCSRA, 0xC0);
    check(seen_input != NULL && seen_input->type == AVR_ADC_SINGLE_ENDED && seen_input->channel == 1 &&
              adc_result(chip, M_ADCL, M_ADCH) == 0x155,
          "adc: on_read gets ADC1's input and its result is the conversion's");
    avr_chip_destroy(chip);

    /* An 8-reference config (avr8js's, REFS2 in ADMUX bit 3) with the 2.56 V
       reference at index 4: ADMUX = REFS2, channel 8 single-ended on ADC0. */
    {
        AvrCpu *cpu = avr_cpu_create(32768, 2048);
        AvrAdcConfig config = avr_layout_find("atmega328")->adc;
        AvrAdc adc;
        int i;

        config.reference_count = 8;
        config.adc_references[4] = AVR_ADC_INTERNAL_2V56;
        config.mux_channels[8].type = AVR_ADC_SINGLE_ENDED;
        config.mux_channels[8].channel = 0;
        avr_adc_init(&adc, cpu, &config);
        adc.channel_volts[0] = 1.28;
        avr_write_data(cpu, config.ADMUX, 0x08, 0xff);
        avr_write_data(cpu, config.ADCSRA, 0xC0, 0xff);
        for (i = 0; i < 400; i++) {
            cpu->cycles++;
            avr_cpu_tick(cpu);
        }
        check((cpu->data[config.ADCL] | (cpu->data[config.ADCH] << 8)) == 512,
              "adc: an 8-reference config: REFS2 picks reference 4 (2.56 V)");
        avr_cpu_destroy(cpu);
    }
}

int main(void) {
    event_cases();
    pc22_cases();
    chip_cases();
    eifr_case();
    timer0_cases();
    timer1_cases();
    twi_cases();
    spi_usart_cases();
    usi_cases();
    adc_cases();
    if (failures > 0) {
        printf("avr_cov_test: %d failures\n", failures);
        return 1;
    }
    printf("avr_cov_test: all passed\n");
    return 0;
}
