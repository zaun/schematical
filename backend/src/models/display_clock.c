/*
 * The `clock` display driver (plan-56-F, PCF1179C; docs/parts-format.md § lcddriver, driver
 * "clock"): a bus-less duplex LCD clock face. It keeps hours, minutes and seconds on a 1 Hz timer
 * of simulated time, shows them as four 7-segment digits (the cells and glyphs come from the
 * device), flashes the colon, and runs the two push-button inputs: a debounce, one step, then
 * after a first-repeat delay four steps a second; both buttons together light every segment and,
 * released together, reset the clock (unless DATA is held low). TS low stops the counters and
 * makes the button times 64 times shorter. The lcd waveforms themselves are display_out.c's.
 *
 * Time: 24-hour counters (hour 0-23); the 12-hour face is derived (12, 1 ... 11, AM or PM).
 */

#include <stdlib.h>

#include "display_clock.h"
#include "converter_int.h"
#include "display_out.h"
#include "logic_alloc.h"

enum {
    CK_TICK = 1, CK_DEBOUNCE = 2, CK_REPEAT = 3
};

typedef struct {
    int raw;                /* the pin is low (pressed) and the buttons are enabled */
    int pressed;            /* past the debounce */
    uint32_t gen;
} ClkKey;

typedef struct ClockState {
    int sec, min, hour;
    int h12;                /* 12-hour face */
    int test;               /* both buttons held: every segment on */
    int settled;            /* one tick has passed: MODE no longer sets the start-up time */
    uint32_t tick_gen;
    ClkKey key[2];
} ClockState;

static uint32_t tid(unsigned kind, unsigned index, uint32_t gen) {
    return CV_TM_DISP | ((uint32_t)kind << 24) | ((uint32_t)(index & 0xFFu) << 16) | (gen & 0xFFFFu);
}

static int pin_is(const CvState *s, int name, LogicValue want, LogicValue z_reads) {
    return name != CV_NONE && cv_level(s, cv_pin(s, name), z_reads) == want;
}

static int fast(const CvState *s, const ClockCfg *k) {
    return pin_is(s, k->fast_pin, LOGIC_0, LOGIC_1);
}

static uint64_t scaled(const CvState *s, const ClockCfg *k, uint64_t ps) {
    uint64_t d = fast(s, k) && k->fast_div > 1 ? k->fast_div : 1;

    return ps / d > 0 ? ps / d : 1;
}

/* The mode and the start-up time: MODE high is the 12-hour face (1:00 AM), low the 24-hour face
   (0:00); an open MODE starts 24-hour and a reset flips the face. */
static void power_on(CvState *s, const ClockCfg *k, ClockState *c, int is_reset) {
    LogicValue m = k->mode_pin == CV_NONE ? LOGIC_0 : cv_level(s, cv_pin(s, k->mode_pin), LOGIC_Z);

    if (m == LOGIC_1) {
        c->h12 = 1;
    } else if (m == LOGIC_0) {
        c->h12 = 0;
    } else if (is_reset) {
        c->h12 = !c->h12;
    } else {
        c->h12 = 0;
    }
    c->sec = 0;
    c->min = 0;
    c->hour = c->h12 ? 1 : 0;
}

static void schedule_tick(CvState *s, ClockState *c) {
    c->tick_gen++;
    sim_schedule_timer(s->sim, s->inst, tid(CK_TICK, 0, c->tick_gen), 1000000000000ull);
}

ClockState *clock_new(CvState *s, const ClockCfg *k) {
    ClockState *c = logic_calloc(1, sizeof *c);

    if (c != NULL) {
        power_on(s, k, c, 0);
        schedule_tick(s, c);
    }
    return c;
}

void clock_free(ClockState *c) {
    logic_free(c);
}

static int enabled(const CvState *s, const ClockCfg *k) {
    return k->enable_pin == CV_NONE || pin_is(s, k->enable_pin, LOGIC_1, LOGIC_0);
}

static void step(ClockState *c, const ClockCfg *k, unsigned key) {
    if (k->key_minute[key]) {
        c->min = (c->min + 1) % 60;
        c->sec = 0;
    } else {
        c->hour = (c->hour + 1) % 24;
    }
}

static int pressed_count(const ClockState *c, const ClockCfg *k) {
    unsigned i;
    int n = 0;

    for (i = 0; i < k->key_count; i++) {
        n += c->key[i].pressed;
    }
    return n;
}

/* A key's pin changed, or the enable did: start or cancel its debounce. */
static void key_changed(CvState *s, const ClockCfg *k, ClockState *c, unsigned i) {
    ClkKey *key = &c->key[i];
    int low = pin_is(s, k->key_pin[i], LOGIC_0, LOGIC_1) && enabled(s, k);

    if (low == key->raw) {
        return;
    }
    key->raw = low;
    key->gen++;
    if (low) {
        sim_schedule_timer(s->sim, s->inst, tid(CK_DEBOUNCE, i, key->gen),
                           scaled(s, k, k->debounce_ps));
        return;
    }
    if (key->pressed) {
        key->pressed = 0;
        if (c->test && pressed_count(c, k) == 0) {
            c->test = 0;
            if (k->data_pin == CV_NONE || !pin_is(s, k->data_pin, LOGIC_0, LOGIC_1)) {
                power_on(s, k, c, 1);
                schedule_tick(s, c);
            }
        }
    }
}

void clock_pin(CvState *s, const ClockCfg *k, ClockState *c, size_t pin) {
    unsigned i;

    /* The straps are applied just after the start: until the first tick the clock still takes
       its start-up mode (and 1:00 / 0:00) from MODE. */
    if (!c->settled && k->mode_pin != CV_NONE && cv_pin(s, k->mode_pin) == pin) {
        power_on(s, k, c, 0);
        return;
    }

    if (k->enable_pin != CV_NONE && cv_pin(s, k->enable_pin) == pin) {
        for (i = 0; i < k->key_count; i++) {
            key_changed(s, k, c, i);
        }
        return;
    }
    for (i = 0; i < k->key_count; i++) {
        if (cv_pin(s, k->key_pin[i]) == pin) {
            key_changed(s, k, c, i);
        }
    }
}

/* A clock timer (an id with kind bits: every one is the clock's own); returns 1. */
int clock_timer(CvState *s, const ClockCfg *k, ClockState *c, uint32_t id) {
    unsigned kind = (id >> 24) & 0xFu, i = (id >> 16) & 0xFFu;
    uint32_t gen = id & 0xFFFFu;

    if (kind == CK_TICK) {
        if (gen != (c->tick_gen & 0xFFFFu)) {
            return 1;
        }
        c->settled = 1;
        if (!fast(s, k)) {
            if (++c->sec >= 60) {
                c->sec = 0;
                if (++c->min >= 60) {
                    c->min = 0;
                    c->hour = (c->hour + 1) % 24;
                }
            }
        }
        schedule_tick(s, c);
        return 1;
    }
    /* CK_DEBOUNCE or CK_REPEAT of key i < key_count: tid() makes these three kinds only, each
       for a key of this device. */
    {
        ClkKey *key = &c->key[i];

        if (gen != (key->gen & 0xFFFFu) || !key->raw) {
            return 1;
        }
        if (kind == CK_DEBOUNCE) {
            unsigned o;
            int other_down = 0;

            key->pressed = 1;
            for (o = 0; o < k->key_count; o++) {
                other_down |= o != i && c->key[o].raw;   /* held, debounced or not (overlapping) */
            }
            if (other_down || pressed_count(c, k) >= 2) {
                for (o = 0; o < k->key_count; o++) {
                    c->key[o].pressed |= c->key[o].raw;
                }
                c->test = 1;                /* both held: segment test, nothing steps */
                return 1;
            }
            step(c, k, i);
            /* the first repeat comes after the long delay; each later one after the short */
            sim_schedule_timer(s->sim, s->inst, tid(CK_REPEAT, i, key->gen), scaled(s, k, k->first_ps));
            return 1;
        }
        if (!c->test) {             /* both held: nothing steps */
            step(c, k, i);
            sim_schedule_timer(s->sim, s->inst, tid(CK_REPEAT, i, key->gen), scaled(s, k, k->repeat_ps));
        }
        return 1;
    }
}

/* ---- What is lit ----------------------------------------------------------------------------- */

static int hour_shown(const ClockState *c) {
    int h = c->hour;

    if (c->h12) {
        h %= 12;
        return h == 0 ? 12 : h;
    }
    return h;
}

static unsigned value_of(const ClockState *c, ClkValue v) {
    switch (v) {
    case CLK_HOUR_TENS:
        return (unsigned)hour_shown(c) / 10u;
    case CLK_HOUR_UNITS:
        return (unsigned)hour_shown(c) % 10u;
    case CLK_MINUTE_TENS:
        return (unsigned)c->min / 10u;
    default:
        return (unsigned)c->min % 10u;
    }
}

static int same(const ClkCell *a, unsigned seg, unsigned bp) {
    return a->seg >= 0 && (unsigned)a->seg == seg && a->bp == bp;
}

/* Is the cell (segment pin index `seg`, backplane `bp`) lit now? */
int clock_lit(const CvState *s, const ClockCfg *k, const ClockState *c, unsigned seg, unsigned bp) {
    unsigned d, j;

    if (c->test) {
        return 1;
    }
    for (d = 0; d < k->digit_count; d++) {
        const ClkDigit *g = &k->digit[d];
        unsigned v = value_of(c, g->value);

        if (v >= g->glyph_count) {
            continue;
        }
        for (j = 0; j < CLK_SEGS; j++) {
            if (((g->glyph[v] >> j) & 1u) && same(&g->seg[j], seg, bp)) {
                return 1;
            }
        }
    }
    if (same(&k->colon, seg, bp)) {
        return !pin_is(s, k->flash_pin, LOGIC_1, LOGIC_0) || (c->sec & 1);
    }
    if (c->h12) {
        if (same(&k->am, seg, bp)) {
            return c->hour < 12;
        }
        if (same(&k->pm, seg, bp)) {
            return c->hour >= 12;
        }
    }
    return 0;
}

