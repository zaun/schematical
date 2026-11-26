/*
 * Timer/Counter1 of the ATtiny25/45/85: an 8-bit timer with its own
 * prescaler (CK to CK/16384), a top register (OCR1C) and two compare
 * outputs. A C port of avr8js 0.21.1's src/peripherals/timer-attiny.ts
 * (Copyright (c) Uri Shaked and contributors, MIT; see LICENSE-avr8js).
 *
 * As in avr8js the count is brought up to date lazily, from the CPU's cycle
 * counter, by a clock event and by reads of TCNT1.
 *
 * Where avr8js disagrees with the datasheet (Atmel 2586, chapter 12) the
 * code follows the datasheet; each place has a comment, and cpu->exact
 * switches it back to avr8js's behavior for the oracle tests.
 *
 * Not modeled, as in avr8js: the 64 MHz PLL as the timer's clock (PCKE: the
 * timer always counts the CPU clock; PLOCK reads 1 once PLLE is set, so
 * firmware that waits for the lock goes on) and the dead time generator.
 */

#include <string.h>

#include "avr.h"

/* TCCR1 bits. */
enum { CTC1 = 0x80, PWM1A = 0x40, CS_MASK = 0x0f };
/* GTCCR bits. */
enum { PWM1B = 0x40, FOC1B = 0x08, FOC1A = 0x04, PSR1 = 0x02 };
/* PLLCSR bits. */
enum { PLOCK = 0x01, PLLE = 0x02 };

#define HOOK_ARGS void *ctx, uint8_t value, uint8_t old_value, uint16_t addr, uint8_t mask
#define HOOK_UNUSED (void)old_value; (void)addr; (void)mask

static uint8_t tccr1(const AvrTinyTimer1 *timer) { return timer->cpu->data[timer->config.TCCR1]; }
static uint8_t gtccr(const AvrTinyTimer1 *timer) { return timer->cpu->data[timer->config.GTCCR]; }
static int ctc_mode(const AvrTinyTimer1 *timer) { return (tccr1(timer) & CTC1) != 0; }
static int pwm_a(const AvrTinyTimer1 *timer) { return (tccr1(timer) & PWM1A) != 0; }
static int pwm_b(const AvrTinyTimer1 *timer) { return (gtccr(timer) & PWM1B) != 0; }
static int com_a(const AvrTinyTimer1 *timer) { return (tccr1(timer) >> 4) & 3; }
static int com_b(const AvrTinyTimer1 *timer) { return (gtccr(timer) >> 4) & 3; }

/* TOP: OCR1C in the CTC and PWM modes, 0xff in the normal mode. */
static int top_of(const AvrTinyTimer1 *timer) {
    if (ctc_mode(timer) || pwm_a(timer) || pwm_b(timer)) {
        return timer->ocr_c;
    }
    return 0xff;
}

static void override_pin(AvrTinyTimer1 *timer, int pin, AvrPinOverride mode) {
    AvrPort *port = avr_cpu_port_at(timer->cpu, timer->config.comp_port_b);

    if (port != NULL) {
        avr_port_override_pin(port, pin, mode);
    }
}

static void update_comp_pin_non_pwm(AvrTinyTimer1 *timer, int channel_b) {
    int com = channel_b ? com_b(timer) : com_a(timer);
    int pin = channel_b ? timer->config.comp_pin_b : timer->config.comp_pin_a;

    /* com is 1-3: every caller checks that it is not 0. */
    override_pin(timer, pin, com == 1 ? AVR_OVERRIDE_TOGGLE
                             : com == 2 ? AVR_OVERRIDE_CLEAR : AVR_OVERRIDE_SET);
}

/* avr8js's PWM compare output: for its up-and-down count. */
static void update_comp_pin_pwm(AvrTinyTimer1 *timer, int channel_b) {
    int com = channel_b ? com_b(timer) : com_a(timer);
    int pin = channel_b ? timer->config.comp_pin_b : timer->config.comp_pin_a;
    int is_set = timer->counting_up == (com == 3);

    switch (com) {
        case 1: override_pin(timer, pin, AVR_OVERRIDE_TOGGLE); break;
        case 2:
        case 3: override_pin(timer, pin, is_set ? AVR_OVERRIDE_SET : AVR_OVERRIDE_CLEAR); break;
        default: break;
    }
}

static void force_compare(AvrTinyTimer1 *timer, int channel_b) {
    if (!channel_b && !pwm_a(timer) && com_a(timer)) {
        update_comp_pin_non_pwm(timer, 0);
    } else if (channel_b && !pwm_b(timer) && com_b(timer)) {
        update_comp_pin_non_pwm(timer, 1);
    }
}

static void update_comp_config(AvrTinyTimer1 *timer) {
    int a = com_a(timer) != 0;
    int b = com_b(timer) != 0;

    /* OC1x is connected while COM1x is not 00 (datasheet table 12-1);
       writing TCCR1 or GTCCR does not change its level otherwise. avr8js
       connects it again on every write, which puts the PORT bit back on the
       pin and undoes a compare match or FOC1x; `exact` keeps that. */
    if (timer->cpu->exact || a != timer->connected_a) {
        timer->connected_a = a;
        override_pin(timer, timer->config.comp_pin_a, a ? AVR_OVERRIDE_ENABLE : AVR_OVERRIDE_NONE);
    }
    if (timer->cpu->exact || b != timer->connected_b) {
        timer->connected_b = b;
        override_pin(timer, timer->config.comp_pin_b, b ? AVR_OVERRIDE_ENABLE : AVR_OVERRIDE_NONE);
    }
    if (!timer->cpu->exact) {
        /* The inverted outputs /OC1A and /OC1B are connected in PWM mode
           with COM1x = 01 (datasheet table 12-1). avr8js has no such pins.
           They are touched only when their connection changes: /OC1A's pin
           is also Timer0's OC0A. */
        int a = pwm_a(timer) && com_a(timer) == 1;
        int b = pwm_b(timer) && com_b(timer) == 1;

        if (a != timer->inverted_a) {
            timer->inverted_a = a;
            override_pin(timer, timer->config.comp_pin_a_inverted,
                         a ? AVR_OVERRIDE_ENABLE : AVR_OVERRIDE_NONE);
        }
        if (b != timer->inverted_b) {
            timer->inverted_b = b;
            override_pin(timer, timer->config.comp_pin_b_inverted,
                         b ? AVR_OVERRIDE_ENABLE : AVR_OVERRIDE_NONE);
        }
    }
}

/* avr8js's PWM count: up to TOP and down again. */
static int phase_pwm_count(AvrTinyTimer1 *timer, int value, int64_t delta) {
    AvrCpu *cpu = timer->cpu;
    int top = top_of(timer);

    while (delta > 0) {
        if (timer->counting_up) {
            value++;
            if (value >= top) {
                value = top;
                timer->counting_up = 0;
            }
        } else {
            value--;
            if (value <= 0) {
                value = 0;
                timer->counting_up = 1;
                avr_set_interrupt_flag(cpu, &timer->OVF);
            }
        }
        if (!timer->tcnt_updated) {
            if (value == timer->ocr_a) {
                avr_set_interrupt_flag(cpu, &timer->OCFA);
                update_comp_pin_pwm(timer, 0);
            }
            if (value == timer->ocr_b) {
                avr_set_interrupt_flag(cpu, &timer->OCFB);
                update_comp_pin_pwm(timer, 1);
            }
        }
        delta--;
    }
    return value & 0xff;
}

static void timer_updated(AvrTinyTimer1 *timer, int value, int prev_value) {
    int ocr_a = timer->ocr_a, ocr_b = timer->ocr_b;
    int overflow = prev_value > value;

    if (((prev_value < ocr_a || overflow) && value >= ocr_a) || (prev_value < ocr_a && overflow)) {
        avr_set_interrupt_flag(timer->cpu, &timer->OCFA);
        if (com_a(timer) && !pwm_a(timer)) {
            update_comp_pin_non_pwm(timer, 0);
        }
    }
    if (((prev_value < ocr_b || overflow) && value >= ocr_b) || (prev_value < ocr_b && overflow)) {
        avr_set_interrupt_flag(timer->cpu, &timer->OCFB);
        if (com_b(timer) && !pwm_b(timer)) {
            update_comp_pin_non_pwm(timer, 1);
        }
    }
}

/* ---- The datasheet's PWM mode ------------------------------------------------ */

/* A PWM channel's output and its inverse: `level` on OC1x. */
static void pwm_output(AvrTinyTimer1 *timer, int channel_b, int level) {
    int com = channel_b ? com_b(timer) : com_a(timer);
    int pin = channel_b ? timer->config.comp_pin_b : timer->config.comp_pin_a;
    int inverted = channel_b ? timer->config.comp_pin_b_inverted : timer->config.comp_pin_a_inverted;

    if (com == 0) {
        return;
    }
    override_pin(timer, pin, level ? AVR_OVERRIDE_SET : AVR_OVERRIDE_CLEAR);
    if (com == 1) {
        override_pin(timer, inverted, level ? AVR_OVERRIDE_CLEAR : AVR_OVERRIDE_SET);
    }
}

/*
 * One count in PWM mode as the datasheet has it (12.2, tables 12-1 and
 * 12-3): the counter runs from 0 up to OCR1C and starts again at 0, which
 * sets TOV1. OC1x is set at 0 and cleared at the compare match (COM1x = 01
 * or 10; the other way round for 11), and OCR1A and OCR1B change only when
 * the counter starts again. An OCR1x equal to OCR1C leaves the output on
 * for the whole period.
 *
 * avr8js counts up to OCR1C and down again instead (phase_pwm_count), which
 * halves the PWM frequency; `exact` keeps that for the oracle tests.
 */
static void fast_pwm_tick(AvrTinyTimer1 *timer) {
    AvrCpu *cpu = timer->cpu;
    int top = top_of(timer);
    int channel;

    if (timer->tcnt >= top) {
        timer->tcnt = 0;
        timer->ocr_a = timer->ocr_a_next;
        timer->ocr_b = timer->ocr_b_next;
        avr_set_interrupt_flag(cpu, &timer->OVF);
        if (pwm_a(timer)) {
            pwm_output(timer, 0, com_a(timer) != 3);
        }
        if (pwm_b(timer)) {
            pwm_output(timer, 1, com_b(timer) != 3);
        }
    } else {
        timer->tcnt++;
    }
    for (channel = 0; channel < 2; channel++) {
        int ocr = channel ? timer->ocr_b : timer->ocr_a;
        int pwm = channel ? pwm_b(timer) : pwm_a(timer);
        int com = channel ? com_b(timer) : com_a(timer);

        if (timer->tcnt != ocr) {
            continue;
        }
        avr_set_interrupt_flag(cpu, channel ? &timer->OCFB : &timer->OCFA);
        if (pwm) {
            if (ocr != top) {
                pwm_output(timer, channel, com == 3);
            }
        } else if (com) {
            update_comp_pin_non_pwm(timer, channel);
        }
    }
}

/* ---- Counting ------------------------------------------------------------------ */

static void count(AvrTinyTimer1 *timer, int reschedule);

static void count_event(void *ctx) {
    count(ctx, 1);
}

static void count(AvrTinyTimer1 *timer, int reschedule) {
    AvrCpu *cpu = timer->cpu;
    int divider = timer->divider;
    int64_t cycles = (int64_t)cpu->cycles;
    int64_t delta = cycles - timer->last_cycle;

    if (divider && delta >= divider) {
        int64_t counter_delta = delta / divider;
        int pwm = pwm_a(timer) || pwm_b(timer);

        timer->last_cycle += counter_delta * divider;
        if (pwm && !cpu->exact) {
            if (!timer->tcnt_updated) {
                for (; counter_delta > 0; counter_delta--) {
                    fast_pwm_tick(timer);
                }
            }
        } else {
            int val = timer->tcnt;
            int top = top_of(timer);
            int phase_pwm = pwm && !ctc_mode(timer);
            int new_val = phase_pwm ? phase_pwm_count(timer, val, counter_delta)
                                    : (int)((val + counter_delta) % (top + 1));
            int overflow = val + counter_delta > top;

            if (!timer->tcnt_updated) {
                timer->tcnt = new_val;
                if (!phase_pwm) {
                    timer_updated(timer, new_val, val);
                }
            }
            if (!phase_pwm && overflow) {
                avr_set_interrupt_flag(cpu, &timer->OVF);
            }
        }
    }
    if (timer->tcnt_updated) {
        timer->tcnt = timer->tcnt_next;
        timer->tcnt_updated = 0;
    }
    if (timer->update_divider) {
        int new_divider = timer->config.dividers[tccr1(timer) & CS_MASK];

        timer->last_cycle = new_divider ? (int64_t)cpu->cycles : 0;
        timer->update_divider = 0;
        timer->divider = new_divider;
        if (new_divider) {
            avr_add_clock_event(cpu, count_event, timer,
                                (double)(timer->last_cycle + new_divider - (int64_t)cpu->cycles));
        }
        return;
    }
    if (reschedule && divider) {
        avr_add_clock_event(cpu, count_event, timer,
                            (double)(timer->last_cycle + divider - (int64_t)cpu->cycles));
    }
}

/* ---- Register hooks -------------------------------------------------------------- */

static uint8_t tcnt_read(void *ctx, uint16_t addr) {
    AvrTinyTimer1 *timer = ctx;

    (void)addr;
    count(timer, 0);
    return timer->cpu->data[timer->config.TCNT1] = (uint8_t)timer->tcnt;
}

static int tcnt_write(HOOK_ARGS) {
    AvrTinyTimer1 *timer = ctx;

    HOOK_UNUSED;
    timer->tcnt_next = value;
    timer->counting_up = 1;
    timer->tcnt_updated = 1;
    avr_update_clock_event(timer->cpu, count_event, timer, 0);
    if (timer->divider) {
        timer_updated(timer, timer->tcnt_next, timer->tcnt_next);
    }
    return 0;
}

static int ocr1a_write(HOOK_ARGS) {
    AvrTinyTimer1 *timer = ctx;

    HOOK_UNUSED;
    timer->ocr_a_next = value;
    /* The datasheet (12.3.5): in PWM mode a new OCR1A waits in a buffer
       until the counter reaches OCR1C. avr8js uses it at once; `exact`
       keeps that for the oracle tests. */
    if (timer->cpu->exact || !pwm_a(timer)) {
        timer->ocr_a = value;
    }
    return 0;
}

static int ocr1b_write(HOOK_ARGS) {
    AvrTinyTimer1 *timer = ctx;

    HOOK_UNUSED;
    timer->ocr_b_next = value;
    if (timer->cpu->exact || !pwm_b(timer)) {   /* as OCR1A */
        timer->ocr_b = value;
    }
    return 0;
}

static int ocr1c_write(HOOK_ARGS) {
    AvrTinyTimer1 *timer = ctx;

    HOOK_UNUSED;
    timer->ocr_c = value;
    return 0;
}

static int tccr1_write(HOOK_ARGS) {
    AvrTinyTimer1 *timer = ctx;
    AvrCpu *cpu = timer->cpu;

    HOOK_UNUSED;
    cpu->data[timer->config.TCCR1] = value;
    timer->update_divider = 1;
    avr_clear_clock_event(cpu, count_event, timer);
    avr_add_clock_event(cpu, count_event, timer, 0);
    update_comp_config(timer);
    return 1;
}

/* GTCCR, TIFR and TIMSK are shared with Timer0: its hook runs too. */
static int gtccr_write(HOOK_ARGS) {
    AvrTinyTimer1 *timer = ctx;
    AvrCpu *cpu = timer->cpu;

    if (value & FOC1A) {
        force_compare(timer, 0);
    }
    if (value & FOC1B) {
        force_compare(timer, 1);
    }
    if (value & PSR1) {
        timer->last_cycle = (int64_t)cpu->cycles;
    }
    value &= (uint8_t)~(FOC1A | FOC1B | PSR1);
    if (timer->gtccr_hook.fn != NULL) {
        timer->gtccr_hook.fn(timer->gtccr_hook.ctx, value, old_value, addr, mask);
    } else {
        cpu->data[timer->config.GTCCR] = value;
    }
    update_comp_config(timer);
    return 1;
}

static int tifr_write(HOOK_ARGS) {
    AvrTinyTimer1 *timer = ctx;
    AvrCpu *cpu = timer->cpu;

    if (timer->tifr_hook.fn != NULL) {
        timer->tifr_hook.fn(timer->tifr_hook.ctx, value, old_value, addr, mask);
    } else {
        cpu->data[timer->config.TIFR] = value;
    }
    avr_clear_interrupt_by_flag(cpu, &timer->OVF, value);
    avr_clear_interrupt_by_flag(cpu, &timer->OCFA, value);
    avr_clear_interrupt_by_flag(cpu, &timer->OCFB, value);
    return 1;
}

static int timsk_write(HOOK_ARGS) {
    AvrTinyTimer1 *timer = ctx;
    AvrCpu *cpu = timer->cpu;

    if (timer->timsk_hook.fn != NULL) {
        timer->timsk_hook.fn(timer->timsk_hook.ctx, value, old_value, addr, mask);
    }
    avr_update_interrupt_enable(cpu, &timer->OVF, value);
    avr_update_interrupt_enable(cpu, &timer->OCFA, value);
    avr_update_interrupt_enable(cpu, &timer->OCFB, value);
    return 0;
}

/* Not in avr8js. The PLL is not modeled, but firmware that turns it on
   waits for PLOCK: it reads 1 while PLLE is set. The register's byte in
   memory is left alone, so the oracle tests see what avr8js has. */
static uint8_t pllcsr_read(void *ctx, uint16_t addr) {
    AvrTinyTimer1 *timer = ctx;
    uint8_t value = timer->cpu->data[addr];

    if (!timer->cpu->exact && (value & PLLE)) {
        value |= PLOCK;
    }
    return value;
}

static AvrWriteHook hook(AvrTinyTimer1 *timer, uint16_t addr, AvrWriteFn fn) {
    AvrWriteHook previous = {NULL, NULL};

    if (addr != 0 && addr < timer->cpu->data_size) {
        previous = timer->cpu->write_hooks[addr];
        timer->cpu->write_hooks[addr].fn = fn;
        timer->cpu->write_hooks[addr].ctx = timer;
    }
    return previous;
}

static void set_interrupt(AvrInterrupt *interrupt, uint8_t address, uint16_t flag_register,
                          uint8_t flag_mask, uint16_t enable_register, uint8_t enable_mask) {
    memset(interrupt, 0, sizeof(*interrupt));
    interrupt->address = address;
    interrupt->flag_register = flag_register;
    interrupt->flag_mask = flag_mask;
    interrupt->enable_register = enable_register;
    interrupt->enable_mask = enable_mask;
}

void avr_tiny_timer1_init(AvrTinyTimer1 *timer, AvrCpu *cpu, const AvrTinyTimer1Config *config) {
    memset(timer, 0, sizeof(*timer));
    timer->cpu = cpu;
    timer->config = *config;
    timer->counting_up = 1;
    set_interrupt(&timer->OVF, config->ovf_interrupt, config->TIFR, config->TOV1, config->TIMSK,
                  config->TOIE1);
    set_interrupt(&timer->OCFA, config->comp_a_interrupt, config->TIFR, config->OCF1A,
                  config->TIMSK, config->OCIE1A);
    set_interrupt(&timer->OCFB, config->comp_b_interrupt, config->TIFR, config->OCF1B,
                  config->TIMSK, config->OCIE1B);
    if (config->TCNT1 < cpu->data_size) {
        cpu->read_hooks[config->TCNT1].fn = tcnt_read;
        cpu->read_hooks[config->TCNT1].ctx = timer;
    }
    hook(timer, config->TCNT1, tcnt_write);
    hook(timer, config->OCR1A, ocr1a_write);
    hook(timer, config->OCR1B, ocr1b_write);
    hook(timer, config->OCR1C, ocr1c_write);
    hook(timer, config->TCCR1, tccr1_write);
    timer->gtccr_hook = hook(timer, config->GTCCR, gtccr_write);
    timer->tifr_hook = hook(timer, config->TIFR, tifr_write);
    timer->timsk_hook = hook(timer, config->TIMSK, timsk_write);
    if (config->PLLCSR != 0 && config->PLLCSR < cpu->data_size) {
        cpu->read_hooks[config->PLLCSR].fn = pllcsr_read;
        cpu->read_hooks[config->PLLCSR].ctx = timer;
    }
}

void avr_tiny_timer1_reset(AvrTinyTimer1 *timer) {
    timer->last_cycle = 0;
    timer->tcnt = 0;
    timer->tcnt_next = 0;
    timer->tcnt_updated = 0;
    timer->ocr_a = 0;
    timer->ocr_b = 0;
    timer->ocr_c = 0;
    timer->ocr_a_next = 0;
    timer->ocr_b_next = 0;
    timer->divider = 0;
    timer->update_divider = 0;
    timer->counting_up = 1;
    timer->inverted_a = 0;
    timer->inverted_b = 0;
    timer->connected_a = 0;
    timer->connected_b = 0;
}
