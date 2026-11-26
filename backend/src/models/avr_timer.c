/*
 * AVR 8- and 16-bit timers. A C port of avr8js 0.21.1's
 * src/peripherals/timer.ts (Copyright (c) Uri Shaked and contributors, MIT;
 * see LICENSE-avr8js). plan-62-B.
 *
 * As in avr8js the count is brought up to date lazily, from the CPU's cycle
 * counter, by a clock event and by reads of TCNT; a compare output changes
 * its pin when that update runs, between instructions.
 */

#include <math.h>
#include <string.h>

#include "avr.h"

enum { EXTERNAL_FALLING_EDGE = 6, EXTERNAL_RISING_EDGE = 7 };

enum { MODE_NORMAL, MODE_PWM_PHASE_CORRECT, MODE_CTC, MODE_FAST_PWM,
       MODE_PWM_PHASE_FREQUENCY_CORRECT, MODE_RESERVED };

enum { TOV_MAX, TOV_TOP, TOV_BOTTOM };
enum { OCR_IMMEDIATE, OCR_TOP, OCR_BOTTOM };

enum { TOP_OCRA = 1, TOP_ICR = 2 };

/* OCnA toggles on compare match in this PWM mode. */
enum { OC_TOGGLE = 1 };

typedef struct { int mode, top, ocr_update, tov_update, flags; } WgmConfig;

static const WgmConfig WGM_8BIT[8] = {
    /*0*/ {MODE_NORMAL, 0xff, OCR_IMMEDIATE, TOV_MAX, 0},
    /*1*/ {MODE_PWM_PHASE_CORRECT, 0xff, OCR_TOP, TOV_BOTTOM, 0},
    /*2*/ {MODE_CTC, TOP_OCRA, OCR_IMMEDIATE, TOV_MAX, 0},
    /*3*/ {MODE_FAST_PWM, 0xff, OCR_BOTTOM, TOV_MAX, 0},
    /*4*/ {MODE_RESERVED, 0xff, OCR_IMMEDIATE, TOV_MAX, 0},
    /*5*/ {MODE_PWM_PHASE_CORRECT, TOP_OCRA, OCR_TOP, TOV_BOTTOM, OC_TOGGLE},
    /*6*/ {MODE_RESERVED, 0xff, OCR_IMMEDIATE, TOV_MAX, 0},
    /*7*/ {MODE_FAST_PWM, TOP_OCRA, OCR_BOTTOM, TOV_TOP, OC_TOGGLE},
};

/* Table 16-4 in the datasheet. */
static const WgmConfig WGM_16BIT[16] = {
    /*0 */ {MODE_NORMAL, 0xffff, OCR_IMMEDIATE, TOV_MAX, 0},
    /*1 */ {MODE_PWM_PHASE_CORRECT, 0x00ff, OCR_TOP, TOV_BOTTOM, 0},
    /*2 */ {MODE_PWM_PHASE_CORRECT, 0x01ff, OCR_TOP, TOV_BOTTOM, 0},
    /*3 */ {MODE_PWM_PHASE_CORRECT, 0x03ff, OCR_TOP, TOV_BOTTOM, 0},
    /*4 */ {MODE_CTC, TOP_OCRA, OCR_IMMEDIATE, TOV_MAX, 0},
    /*5 */ {MODE_FAST_PWM, 0x00ff, OCR_BOTTOM, TOV_TOP, 0},
    /*6 */ {MODE_FAST_PWM, 0x01ff, OCR_BOTTOM, TOV_TOP, 0},
    /*7 */ {MODE_FAST_PWM, 0x03ff, OCR_BOTTOM, TOV_TOP, 0},
    /*8 */ {MODE_PWM_PHASE_FREQUENCY_CORRECT, TOP_ICR, OCR_BOTTOM, TOV_BOTTOM, 0},
    /*9 */ {MODE_PWM_PHASE_FREQUENCY_CORRECT, TOP_OCRA, OCR_BOTTOM, TOV_BOTTOM, OC_TOGGLE},
    /*10*/ {MODE_PWM_PHASE_CORRECT, TOP_ICR, OCR_TOP, TOV_BOTTOM, 0},
    /*11*/ {MODE_PWM_PHASE_CORRECT, TOP_OCRA, OCR_TOP, TOV_BOTTOM, OC_TOGGLE},
    /*12*/ {MODE_CTC, TOP_ICR, OCR_IMMEDIATE, TOV_MAX, 0},
    /*13*/ {MODE_RESERVED, 0xffff, OCR_IMMEDIATE, TOV_MAX, 0},
    /*14*/ {MODE_FAST_PWM, TOP_ICR, OCR_BOTTOM, TOV_TOP, OC_TOGGLE},
    /*15*/ {MODE_FAST_PWM, TOP_OCRA, OCR_BOTTOM, TOV_TOP, OC_TOGGLE},
};

/* Force Output Compare bits. */
enum { FOCA = 1 << 7, FOCB = 1 << 6, FOCC = 1 << 5 };

static void count_event(void *ctx);
static void count(AvrTimer *timer, int reschedule, int external);

static AvrPinOverride comp_to_override(int comp) {
    switch (comp) {
        case 1: return AVR_OVERRIDE_TOGGLE;
        case 2: return AVR_OVERRIDE_CLEAR;
        case 3: return AVR_OVERRIDE_SET;
        default: return AVR_OVERRIDE_ENABLE;
    }
}

static int timer_top(const AvrTimer *timer) {
    switch (timer->top_value) {
        case TOP_OCRA: return timer->ocr_a;
        case TOP_ICR: return timer->icr;
        default: return timer->top_value;
    }
}

static int ocr_mask(const AvrTimer *timer) {
    switch (timer->top_value) {
        case TOP_OCRA:
        case TOP_ICR: return 0xffff;
        default: return timer->top_value;
    }
}

static int timer_cs(const AvrTimer *timer) {
    return timer->cpu->data[timer->config.TCCRB] & 7;
}

static int timer_wgm(const AvrTimer *timer) {
    int mask = timer->config.bits == 16 ? 0x18 : 0x8;

    return ((timer->cpu->data[timer->config.TCCRB] & mask) >> 1) |
           (timer->cpu->data[timer->config.TCCRA] & 3);
}

static void update_comp(AvrTimer *timer, char pin_name, AvrPinOverride value) {
    const AvrTimerConfig *config = &timer->config;
    uint16_t port_address = pin_name == 'A' ? config->comp_port_a
                            : pin_name == 'B' ? config->comp_port_b : config->comp_port_c;
    int pin = pin_name == 'A' ? config->comp_pin_a
              : pin_name == 'B' ? config->comp_pin_b : config->comp_pin_c;
    AvrPort *port = avr_cpu_port_at(timer->cpu, port_address);

    if (port != NULL) {
        avr_port_override_pin(port, pin, value);
    }
}

static void update_comp_pin(AvrTimer *timer, int comp_value, char pin_name, int bottom) {
    AvrPinOverride new_value = AVR_OVERRIDE_NONE;
    int inverting_mode = comp_value == 3;
    int is_set = (timer->counting_up != 0) == inverting_mode;

    switch (timer->timer_mode) {
        case MODE_NORMAL:
        case MODE_CTC:
            new_value = comp_to_override(comp_value);
            break;
        case MODE_FAST_PWM:
            if (comp_value == 1) {
                new_value = bottom ? AVR_OVERRIDE_NONE : AVR_OVERRIDE_TOGGLE;
            } else {
                new_value = inverting_mode != (bottom != 0) ? AVR_OVERRIDE_SET : AVR_OVERRIDE_CLEAR;
            }
            break;
        case MODE_PWM_PHASE_CORRECT:
        case MODE_PWM_PHASE_FREQUENCY_CORRECT:
            if (comp_value == 1) {
                new_value = AVR_OVERRIDE_TOGGLE;
            } else {
                new_value = is_set ? AVR_OVERRIDE_SET : AVR_OVERRIDE_CLEAR;
            }
            break;
    }
    if (new_value != AVR_OVERRIDE_NONE) {
        update_comp(timer, pin_name, new_value);
    }
}

static void update_wgm_config(AvrTimer *timer) {
    const WgmConfig *wgm = timer->config.bits == 16 ? &WGM_16BIT[timer_wgm(timer)]
                                                    : &WGM_8BIT[timer_wgm(timer)];
    uint8_t tccra = timer->cpu->data[timer->config.TCCRA];
    int pwm_mode, prev;

    timer->timer_mode = wgm->mode;
    timer->top_value = wgm->top;
    timer->ocr_update_mode = wgm->ocr_update;
    timer->tov_update_mode = wgm->tov_update;
    pwm_mode = wgm->mode == MODE_FAST_PWM || wgm->mode == MODE_PWM_PHASE_CORRECT ||
               wgm->mode == MODE_PWM_PHASE_FREQUENCY_CORRECT;

    prev = timer->comp_a;
    timer->comp_a = (tccra >> 6) & 3;
    if (timer->comp_a == 1 && pwm_mode && !(wgm->flags & OC_TOGGLE)) {
        timer->comp_a = 0;
    }
    if ((prev != 0) != (timer->comp_a != 0)) {
        update_comp(timer, 'A', timer->comp_a ? AVR_OVERRIDE_ENABLE : AVR_OVERRIDE_NONE);
    }

    prev = timer->comp_b;
    timer->comp_b = (tccra >> 4) & 3;
    if (timer->comp_b == 1 && pwm_mode) {
        timer->comp_b = 0;   /* reserved, according to the datasheet */
    }
    if ((prev != 0) != (timer->comp_b != 0)) {
        update_comp(timer, 'B', timer->comp_b ? AVR_OVERRIDE_ENABLE : AVR_OVERRIDE_NONE);
    }

    if (timer->has_ocr_c) {
        prev = timer->comp_c;
        timer->comp_c = (tccra >> 2) & 3;
        if (timer->comp_c == 1 && pwm_mode) {
            timer->comp_c = 0;   /* reserved, according to the datasheet */
        }
        if ((prev != 0) != (timer->comp_c != 0)) {
            update_comp(timer, 'C', timer->comp_c ? AVR_OVERRIDE_ENABLE : AVR_OVERRIDE_NONE);
        }
    }
}

static void load_ocr(AvrTimer *timer) {
    timer->ocr_a = timer->next_ocr_a;
    timer->ocr_b = timer->next_ocr_b;
    timer->ocr_c = timer->next_ocr_c;
}

static void compare_match(AvrTimer *timer, char pin_name) {
    if (pin_name == 'A') {
        avr_set_interrupt_flag(timer->cpu, &timer->OCFA);
        if (timer->comp_a) {
            update_comp_pin(timer, timer->comp_a, 'A', 0);
        }
    } else if (pin_name == 'B') {
        avr_set_interrupt_flag(timer->cpu, &timer->OCFB);
        if (timer->comp_b) {
            update_comp_pin(timer, timer->comp_b, 'B', 0);
        }
    } else {
        avr_set_interrupt_flag(timer->cpu, &timer->OCFC);
        if (timer->comp_c) {
            update_comp_pin(timer, timer->comp_c, 'C', 0);
        }
    }
}

static int phase_pwm_count(AvrTimer *timer, int value, double delta) {
    /* avr8js reads these once, before the loop. */
    const int ocr_a = timer->ocr_a, ocr_b = timer->ocr_b, ocr_c = timer->ocr_c;
    const int top = timer_top(timer);
    const int tcnt_updated = timer->tcnt_updated;

    if (!value && !top) {
        delta = 0;
        if (timer->ocr_update_mode == OCR_TOP) {
            load_ocr(timer);
        }
    }
    while (delta > 0) {
        if (timer->counting_up) {
            value++;
            if (value == top && !tcnt_updated) {
                timer->counting_up = 0;
                if (timer->ocr_update_mode == OCR_TOP) {
                    load_ocr(timer);
                }
            }
        } else {
            value--;
            if (!value && !tcnt_updated) {
                timer->counting_up = 1;
                avr_set_interrupt_flag(timer->cpu, &timer->OVF);
                if (timer->ocr_update_mode == OCR_BOTTOM) {
                    load_ocr(timer);
                }
            }
        }
        if (!tcnt_updated) {
            if (value == ocr_a) {
                compare_match(timer, 'A');
            }
            if (value == ocr_b) {
                compare_match(timer, 'B');
            }
            if (timer->has_ocr_c && value == ocr_c) {
                compare_match(timer, 'C');
            }
        }
        delta--;
    }
    return value & timer->max;
}

static void timer_updated(AvrTimer *timer, int value, int prev_value) {
    /* Read once, as avr8js does. */
    const int ocr_a = timer->ocr_a, ocr_b = timer->ocr_b, ocr_c = timer->ocr_c;
    const int overflow = prev_value > value;

    if (((prev_value < ocr_a || overflow) && value >= ocr_a) || (prev_value < ocr_a && overflow)) {
        compare_match(timer, 'A');
    }
    if (((prev_value < ocr_b || overflow) && value >= ocr_b) || (prev_value < ocr_b && overflow)) {
        compare_match(timer, 'B');
    }
    if (timer->has_ocr_c &&
        (((prev_value < ocr_c || overflow) && value >= ocr_c) || (prev_value < ocr_c && overflow))) {
        compare_match(timer, 'C');
    }
}

static void external_clock_callback(void *ctx, int value) {
    AvrTimer *timer = ctx;

    if ((value != 0) == (timer->external_clock_rising_edge != 0)) {
        count(timer, 0, 1);
    }
}

static void count(AvrTimer *timer, int reschedule, int external) {
    AvrCpu *cpu = timer->cpu;
    /* Read once, as avr8js does: the values before this call changes them. */
    const int divider = timer->divider;
    const double cycles = (double)cpu->cycles;
    const double delta = cycles - timer->last_cycle;

    if ((divider && delta >= divider) || external) {
        const double counter_delta = external ? 1 : floor(delta / divider);
        const int val = timer->tcnt;
        const int top = timer_top(timer);
        const int phase_pwm = timer->timer_mode == MODE_PWM_PHASE_CORRECT ||
                              timer->timer_mode == MODE_PWM_PHASE_FREQUENCY_CORRECT;
        int new_val, overflow;

        timer->last_cycle += counter_delta * divider;
        new_val = phase_pwm ? phase_pwm_count(timer, val, counter_delta)
                            : (int)fmod(val + counter_delta, (double)top + 1);
        overflow = val + counter_delta > top;
        /* A CPU write overrides (has priority over) all counter clear or count operations. */
        if (!timer->tcnt_updated) {
            timer->tcnt = new_val;
            if (!phase_pwm) {
                timer_updated(timer, new_val, val);
            }
        }

        if (!phase_pwm) {
            if (timer->timer_mode == MODE_FAST_PWM && overflow) {
                const int comp_a = timer->comp_a, comp_b = timer->comp_b;

                if (comp_a) {
                    update_comp_pin(timer, comp_a, 'A', 1);
                }
                if (comp_b) {
                    update_comp_pin(timer, comp_b, 'B', 1);
                }
            }
            if (timer->ocr_update_mode == OCR_BOTTOM && overflow) {
                /* OCR_TOP only occurs in the phase-correct modes (phase_pwm_count). */
                load_ocr(timer);
            }
            /* TOV_BOTTOM only occurs in the phase-correct modes; here it is
               TOV_TOP or TOV_MAX. */
            if (overflow && (timer->tov_update_mode == TOV_TOP || top == timer->max)) {
                avr_set_interrupt_flag(cpu, &timer->OVF);
            }
        }
    }
    if (timer->tcnt_updated) {
        timer->tcnt = timer->tcnt_next;
        timer->tcnt_updated = 0;
        if ((timer->tcnt == 0 && timer->ocr_update_mode == OCR_BOTTOM) ||
            (timer->tcnt == timer_top(timer) && timer->ocr_update_mode == OCR_TOP)) {
            load_ocr(timer);
        }
    }
    if (timer->update_divider) {
        const int cs = timer_cs(timer);
        const int new_divider = timer->config.dividers[cs];

        timer->last_cycle = new_divider ? (double)cpu->cycles : 0;
        timer->update_divider = 0;
        timer->divider = new_divider;
        if (timer->config.external_clock_port && timer->external_clock_port == NULL) {
            timer->external_clock_port = avr_cpu_port_at(cpu, timer->config.external_clock_port);
        }
        if (timer->external_clock_port != NULL) {
            timer->external_clock_port->external_clock[timer->config.external_clock_pin].fn = NULL;
        }
        if (new_divider) {
            avr_add_clock_event(cpu, count_event, timer,
                                timer->last_cycle + new_divider - (double)cpu->cycles);
        } else if (timer->external_clock_port != NULL &&
                   (cs == EXTERNAL_FALLING_EDGE || cs == EXTERNAL_RISING_EDGE)) {
            timer->external_clock_port->external_clock[timer->config.external_clock_pin].fn =
                external_clock_callback;
            timer->external_clock_port->external_clock[timer->config.external_clock_pin].ctx = timer;
            timer->external_clock_rising_edge = cs == EXTERNAL_RISING_EDGE;
        }
        return;
    }
    if (reschedule && divider) {
        avr_add_clock_event(cpu, count_event, timer,
                            timer->last_cycle + divider - (double)cpu->cycles);
    }
}

static void count_event(void *ctx) {
    count(ctx, 1, 0);
}

static void check_force_compare(AvrTimer *timer, int value) {
    if (timer->timer_mode == MODE_FAST_PWM || timer->timer_mode == MODE_PWM_PHASE_CORRECT ||
        timer->timer_mode == MODE_PWM_PHASE_FREQUENCY_CORRECT) {
        /* The FOCnx bits are only active in a non-PWM mode. */
        return;
    }
    if (value & FOCA) {
        update_comp_pin(timer, timer->comp_a, 'A', 0);
    }
    if (value & FOCB) {
        update_comp_pin(timer, timer->comp_b, 'B', 0);
    }
    if (timer->config.comp_port_c && (value & FOCC)) {
        update_comp_pin(timer, timer->comp_c, 'C', 0);
    }
}

/* ---- Hooks ---------------------------------------------------------------- */

static uint8_t tcnt_read(void *ctx, uint16_t addr) {
    AvrTimer *timer = ctx;

    count(timer, 0, 0);
    if (timer->config.bits == 16) {
        timer->cpu->data[addr + 1] = (uint8_t)(timer->tcnt >> 8);
    }
    timer->cpu->data[addr] = (uint8_t)timer->tcnt;
    return timer->cpu->data[addr];
}

#define HOOK_ARGS void *ctx, uint8_t value, uint8_t old_value, uint16_t addr, uint8_t mask
#define HOOK_UNUSED (void)old_value; (void)addr; (void)mask

/* The hooks below that return 0 leave the byte for the core to store, as an
   avr8js hook that returns nothing does. */

static int tcnt_write(HOOK_ARGS) {
    AvrTimer *timer = ctx;

    HOOK_UNUSED;
    timer->tcnt_next = (timer->high_byte_temp << 8) | value;
    timer->counting_up = 1;
    timer->tcnt_updated = 1;
    avr_update_clock_event(timer->cpu, count_event, timer, 0);
    if (timer->divider) {
        timer_updated(timer, timer->tcnt_next, timer->tcnt_next);
    }
    return 0;
}

static int ocra_write(HOOK_ARGS) {
    AvrTimer *timer = ctx;

    HOOK_UNUSED;
    timer->next_ocr_a = (timer->high_byte_temp << 8) | value;
    if (timer->ocr_update_mode == OCR_IMMEDIATE) {
        timer->ocr_a = timer->next_ocr_a;
    }
    return 0;
}

static int ocrb_write(HOOK_ARGS) {
    AvrTimer *timer = ctx;

    HOOK_UNUSED;
    timer->next_ocr_b = (timer->high_byte_temp << 8) | value;
    if (timer->ocr_update_mode == OCR_IMMEDIATE) {
        timer->ocr_b = timer->next_ocr_b;
    }
    return 0;
}

static int ocrc_write(HOOK_ARGS) {
    AvrTimer *timer = ctx;

    HOOK_UNUSED;
    timer->next_ocr_c = (timer->high_byte_temp << 8) | value;
    if (timer->ocr_update_mode == OCR_IMMEDIATE) {
        timer->ocr_c = timer->next_ocr_c;
    }
    return 0;
}

static int icr_write(HOOK_ARGS) {
    AvrTimer *timer = ctx;

    HOOK_UNUSED;
    timer->icr = (timer->high_byte_temp << 8) | value;
    return 0;
}

/* The high byte of TCNT and ICR goes to the temporary register (section 16.3
   of the datasheet). */
static int temp_write(HOOK_ARGS) {
    AvrTimer *timer = ctx;

    HOOK_UNUSED;
    timer->high_byte_temp = value;
    return 0;
}

static int ocr_high_write(HOOK_ARGS) {
    AvrTimer *timer = ctx;

    (void)old_value;
    (void)mask;
    timer->high_byte_temp = (uint8_t)(value & (ocr_mask(timer) >> 8));
    timer->cpu->data[addr] = timer->high_byte_temp;
    return 1;
}

static int tccra_write(HOOK_ARGS) {
    AvrTimer *timer = ctx;

    HOOK_UNUSED;
    timer->cpu->data[timer->config.TCCRA] = value;
    update_wgm_config(timer);
    return 1;
}

static int tccrb_write(HOOK_ARGS) {
    AvrTimer *timer = ctx;

    HOOK_UNUSED;
    if (!timer->config.TCCRC) {
        check_force_compare(timer, value);
        value &= (uint8_t)~(FOCA | FOCB);
    }
    timer->cpu->data[timer->config.TCCRB] = value;
    timer->update_divider = 1;
    avr_clear_clock_event(timer->cpu, count_event, timer);
    avr_add_clock_event(timer->cpu, count_event, timer, 0);
    update_wgm_config(timer);
    return 1;
}

static int tccrc_write(HOOK_ARGS) {
    AvrTimer *timer = ctx;

    HOOK_UNUSED;
    check_force_compare(timer, value);
    return 0;
}

static int tifr_write(HOOK_ARGS) {
    AvrTimer *timer = ctx;

    HOOK_UNUSED;
    timer->cpu->data[timer->config.TIFR] = value;
    avr_clear_interrupt_by_flag(timer->cpu, &timer->OVF, value);
    avr_clear_interrupt_by_flag(timer->cpu, &timer->OCFA, value);
    avr_clear_interrupt_by_flag(timer->cpu, &timer->OCFB, value);
    return 1;
}

static int timsk_write(HOOK_ARGS) {
    AvrTimer *timer = ctx;

    HOOK_UNUSED;
    avr_update_interrupt_enable(timer->cpu, &timer->OVF, value);
    avr_update_interrupt_enable(timer->cpu, &timer->OCFA, value);
    avr_update_interrupt_enable(timer->cpu, &timer->OCFB, value);
    return 0;
}

static void hook(AvrTimer *timer, uint16_t addr, AvrWriteFn fn) {
    if (addr < timer->cpu->data_size) {
        timer->cpu->write_hooks[addr].fn = fn;
        timer->cpu->write_hooks[addr].ctx = timer;
    }
}

static void set_interrupt(AvrInterrupt *interrupt, uint8_t address, uint16_t flag_register,
                          uint8_t flag_mask, uint16_t enable_register, uint8_t enable_mask) {
    interrupt->address = address;
    interrupt->flag_register = flag_register;
    interrupt->flag_mask = flag_mask;
    interrupt->enable_register = enable_register;
    interrupt->enable_mask = enable_mask;
    interrupt->constant = 0;
    interrupt->inverse_flag = 0;
}

void avr_timer_init(AvrTimer *timer, AvrCpu *cpu, const AvrTimerConfig *config) {
    memset(timer, 0, sizeof(*timer));
    timer->cpu = cpu;
    timer->config = *config;
    timer->max = config->bits == 16 ? 0xffff : 0xff;
    timer->has_ocr_c = config->OCRC > 0;
    timer->ocr_update_mode = OCR_IMMEDIATE;
    timer->tov_update_mode = TOV_MAX;
    timer->counting_up = 1;
    set_interrupt(&timer->OVF, config->ovf_interrupt, config->TIFR, config->TOV, config->TIMSK,
                  config->TOIE);
    set_interrupt(&timer->OCFA, config->comp_a_interrupt, config->TIFR, config->OCFA,
                  config->TIMSK, config->OCIEA);
    set_interrupt(&timer->OCFB, config->comp_b_interrupt, config->TIFR, config->OCFB,
                  config->TIMSK, config->OCIEB);
    set_interrupt(&timer->OCFC, config->comp_c_interrupt, config->TIFR, config->OCFC,
                  config->TIMSK, config->OCIEC);

    update_wgm_config(timer);
    if (config->TCNT < cpu->data_size) {
        cpu->read_hooks[config->TCNT].fn = tcnt_read;
        cpu->read_hooks[config->TCNT].ctx = timer;
    }
    hook(timer, config->TCNT, tcnt_write);
    hook(timer, config->OCRA, ocra_write);
    hook(timer, config->OCRB, ocrb_write);
    if (timer->has_ocr_c) {
        hook(timer, config->OCRC, ocrc_write);
    }
    if (config->bits == 16) {
        hook(timer, config->ICR, icr_write);
        hook(timer, config->TCNT + 1, temp_write);
        hook(timer, config->OCRA + 1, ocr_high_write);
        hook(timer, config->OCRB + 1, ocr_high_write);
        if (timer->has_ocr_c) {
            hook(timer, config->OCRC + 1, ocr_high_write);
        }
        hook(timer, config->ICR + 1, temp_write);
    }
    hook(timer, config->TCCRA, tccra_write);
    hook(timer, config->TCCRB, tccrb_write);
    if (config->TCCRC) {
        hook(timer, config->TCCRC, tccrc_write);
    }
    hook(timer, config->TIFR, tifr_write);
    hook(timer, config->TIMSK, timsk_write);
}

void avr_timer_reset(AvrTimer *timer) {
    timer->divider = 0;
    timer->last_cycle = 0;
    timer->ocr_a = 0;
    timer->next_ocr_a = 0;
    timer->ocr_b = 0;
    timer->next_ocr_b = 0;
    timer->ocr_c = 0;
    timer->next_ocr_c = 0;
    timer->icr = 0;
    timer->tcnt = 0;
    timer->tcnt_next = 0;
    timer->tcnt_updated = 0;
    timer->counting_up = 0;
    timer->update_divider = 1;
}
