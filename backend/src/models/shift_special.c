/*
 * shift_special: the shift kind's one-off models (plan-10-H section 4).
 *
 *   ls673         74LS673 / 74LS674 16-bit shift registers (gated falling clock,
 *                 recirculating read, storage or parallel load)
 *   tap64         CD4517B dual 64-stage register with 3-state taps
 *   bus-register  CD4034B 8-stage bidirectional bus register
 *   fifo          CD40105B / 74HC40105 4 x 16 FIFO (timed fall-through)
 *   serial-in     SN74LV8153 serial-to-parallel interface (auto-baud frames)
 *
 * shift.c holds the kind and lists these models (shift.h).
 */

#include <stdlib.h>
#include <string.h>

#include "kind.h"
#include "seq.h"
#include "shift.h"

static int lo(int level) {
    return level < 0 ? 0 : level;
}

static int hi(int level) {
    return level < 0 ? 1 : level;
}

static int and3(int a, int b) {
    if (a == 0 || b == 0) {
        return 0;
    }
    return a == 1 && b == 1 ? 1 : -1;
}

static int or3(int a, int b) {
    if (a == 1 || b == 1) {
        return 1;
    }
    return a == 0 && b == 0 ? 0 : -1;
}

static int not3(int a) {
    return a < 0 ? -1 : !a;
}

static uint64_t mask_of(unsigned n) {
    return n >= 64 ? ~(uint64_t)0 : (((uint64_t)1 << n) - 1);
}

static SeqBits zero_bits(unsigned n) {
    SeqBits b = {0, mask_of(n)};

    return b;
}

static int bind_names(KindContext *ctx, const PartPin *pins, size_t count,
                      const char *const *names, size_t n, size_t *out) {
    size_t i;

    for (i = 0; i < n; i++) {
        if (seq_bind(ctx, pins, count, names[i], &out[i]) != 0) {
            return -1;
        }
    }
    return 0;
}

static void init_outputs(SeqOutput *outputs, const size_t *pins, size_t n) {
    size_t i;

    for (i = 0; i < n; i++) {
        outputs[i].pin = pins[i];
        outputs[i].scheduled = 0;
        outputs[i].driven = LOGIC_X;
    }
}

static void drive_bits(Sim *sim, SimInstance *inst, SeqOutput *outs, size_t n, SeqBits v,
                       int enable, const ShiftTiming *t) {
    size_t i;

    for (i = 0; i < n; i++) {
        shift_drive(sim, inst, &outs[i], seq_gate(seq_bit(v, (unsigned)i), enable), SEQ_PUSH_PULL,
                    t);
    }
}

/* Shift toward the higher stages: stage 1 <- in. */
static SeqBits shift_up(SeqBits cur, unsigned n, int in) {
    SeqBits r;

    r.value = (cur.value << 1) & mask_of(n);
    r.known = (cur.known << 1) & mask_of(n);
    seq_set_bit(&r, 0, seq_value(in));
    return r;
}

/* ---- ls673 ------------------------------------------------------------------- */

static const char *const LS673_KEYS[] = {
    "clock", "chipSelect", "readWrite", "mode", "serialIo", "storageOutputs", "storageClear",
    "parallel", NULL
};

typedef struct {
    const char *clock, *cs, *rw, *mode, *ser;
    const char *const *y;
    size_t y_count;
    SeqControlName st_clear;
    const char *const *p;
    size_t p_count;
} Ls673Device;

typedef struct {
    const Ls673Device *dev;
    size_t clock, cs, rw, mode, ser;
    size_t y[16], p[16];
    SeqControl st_clear;
} Ls673Bound;

typedef struct {
    ShiftHeader h;
    const Ls673Bound *b;
    SeqBits q, y;
    LogicValue last_shift, last_store;
    SeqOutput ser, y_out[16];
} Ls673State;

static void *ls673_load(KindContext *ctx, const cJSON *json, unsigned *units) {
    Ls673Device *d = kind_alloc(ctx, sizeof(*d));

    if (d == NULL || kind_string(ctx, json, "clock", &d->clock) != 0 ||
        kind_string(ctx, json, "chipSelect", &d->cs) != 0 ||
        kind_string(ctx, json, "readWrite", &d->rw) != 0 ||
        kind_string(ctx, json, "mode", &d->mode) != 0 ||
        kind_string(ctx, json, "serialIo", &d->ser) != 0 ||
        seq_control_name(ctx, json, "storageClear", 0, 0, &d->st_clear) != 0) {
        return NULL;
    }
    if (cJSON_GetObjectItemCaseSensitive(json, "storageOutputs") != NULL &&
        seq_names(ctx, json, "storageOutputs", 16, 16, &d->y, &d->y_count) != 0) {
        return NULL;
    }
    if (cJSON_GetObjectItemCaseSensitive(json, "parallel") != NULL &&
        seq_names(ctx, json, "parallel", 16, 16, &d->p, &d->p_count) != 0) {
        return NULL;
    }
    if ((d->y_count > 0) == (d->p_count > 0)) {
        kind_error(ctx, "exactly one of \"storageOutputs\" and \"parallel\" is needed");
        return NULL;
    }
    *units = 1;
    return d;
}

static void *ls673_bind(KindContext *ctx, const void *raw, const PartPin *pins, size_t count) {
    const Ls673Device *d = raw;
    Ls673Bound *b = kind_alloc(ctx, sizeof(*b));

    if (b == NULL || seq_bind(ctx, pins, count, d->clock, &b->clock) != 0 ||
        seq_bind(ctx, pins, count, d->cs, &b->cs) != 0 ||
        seq_bind(ctx, pins, count, d->rw, &b->rw) != 0 ||
        seq_bind(ctx, pins, count, d->mode, &b->mode) != 0 ||
        seq_bind(ctx, pins, count, d->ser, &b->ser) != 0 ||
        bind_names(ctx, pins, count, d->y, d->y_count, b->y) != 0 ||
        bind_names(ctx, pins, count, d->p, d->p_count, b->p) != 0 ||
        seq_bind_control(ctx, pins, count, &d->st_clear, &b->st_clear) != 0) {
        return NULL;
    }
    b->dev = d;
    return b;
}

static ShiftHeader *ls673_create(const void *dev, const void *bound) {
    const Ls673Bound *b = bound;
    Ls673State *s = calloc(1, sizeof(*s));

    (void)dev;
    if (s == NULL) {
        return NULL;
    }
    s->b = b;
    s->q = zero_bits(16);
    s->y = zero_bits(16);
    init_outputs(&s->ser, &b->ser, 1);
    init_outputs(s->y_out, b->y, b->dev->y_count);
    return &s->h;
}

static LogicValue ls673_shift_clock(const Sim *sim, const SimInstance *inst, const Ls673Bound *b) {
    return seq_value(and3(seq_read(sim, inst, b->clock), not3(seq_read(sim, inst, b->cs))));
}

static LogicValue ls673_store_clock(const Sim *sim, const SimInstance *inst, const Ls673Bound *b) {
    return seq_value(and3(and3(seq_read(sim, inst, b->mode), not3(seq_read(sim, inst, b->cs))),
                          not3(seq_read(sim, inst, b->rw))));
}

static void ls673_eval(Sim *sim, SimInstance *inst, Ls673State *s, int at_start) {
    const Ls673Bound *b = s->b;
    const Ls673Device *d = b->dev;
    LogicValue shift_clock = ls673_shift_clock(sim, inst, b);
    LogicValue store_clock = d->y_count > 0 ? ls673_store_clock(sim, inst, b) : LOGIC_0;
    int shift_edge = !at_start && seq_falling(s->last_shift, shift_clock);
    int store_edge = !at_start && seq_rising(s->last_store, store_clock);
    SeqBits q_before = s->q;
    int cs = seq_read(sim, inst, b->cs);
    int rw = seq_read(sim, inst, b->rw);

    s->last_shift = shift_clock;
    s->last_store = store_clock;
    if (d->y_count > 0) {
        int clear = seq_async(sim, inst, &b->st_clear);

        if (store_edge) {
            s->y = q_before;
        }
        if (clear == 1) {
            s->y = zero_bits(16);
        } else if (clear < 0) {
            s->y = shift_merge(s->y, zero_bits(16));
        }
    }
    if (shift_edge) {
        int mode = seq_read(sim, inst, b->mode);
        int q15 = seq_level(seq_bit(q_before, 15));
        SeqBits r = {0, 0};
        int first = 1;
        int w, m;

        for (w = lo(rw); w <= hi(rw); w++) {
            for (m = lo(mode); m <= hi(mode); m++) {
                SeqBits o;

                if (!w) {
                    o = shift_up(q_before, 16, seq_read(sim, inst, b->ser));
                } else if (m) {
                    o = d->y_count > 0 ? s->y : shift_read_bits(sim, inst, b->p, 16);
                } else {
                    o = shift_up(q_before, 16, q15);
                }
                r = first ? o : shift_merge(r, o);
                first = 0;
            }
        }
        s->q = r;
    }
    shift_drive(sim, inst, &s->ser, seq_gate(seq_bit(s->q, 15), and3(not3(cs), rw)),
                SEQ_PUSH_PULL, s->h.timing);
    drive_bits(sim, inst, s->y_out, d->y_count, s->y, 1, s->h.timing);
}

static void ls673_start(Sim *sim, SimInstance *inst, ShiftHeader *h) {
    Ls673State *s = (Ls673State *)h;

    s->last_shift = ls673_shift_clock(sim, inst, s->b);
    s->last_store = s->b->dev->y_count > 0 ? ls673_store_clock(sim, inst, s->b) : LOGIC_0;
    ls673_eval(sim, inst, s, 1);
}

static void ls673_changed(Sim *sim, SimInstance *inst, ShiftHeader *h, size_t pin,
                          LogicValue old_value, LogicValue new_value) {
    (void)pin;
    (void)old_value;
    (void)new_value;
    ls673_eval(sim, inst, (Ls673State *)h, 0);
}

const ShiftModel LS673_MODEL = {
    "ls673", LS673_KEYS, NULL, ls673_load, ls673_bind, ls673_create, ls673_start, ls673_changed,
    NULL, NULL
};

/* ---- tap64 -------------------------------------------------------------------- */

static const char *const TAP64_KEYS[] = {"units", NULL};

typedef struct {
    const char *clock, *data, *we;
    const char *const *taps;
    size_t tap_count;
} Tap64Unit;

typedef struct {
    Tap64Unit units[2];
    size_t unit_count;
} Tap64Device;

typedef struct {
    size_t clock, data, we, taps[4];
} Tap64Pins;

typedef struct {
    Tap64Pins units[2];
    size_t unit_count;
} Tap64Bound;

typedef struct {
    ShiftHeader h;
    const Tap64Bound *b;
    SeqBits stages[2];
    LogicValue last_clock[2];
    SeqOutput taps[2][4];
} Tap64State;

static void *tap64_load(KindContext *ctx, const cJSON *json, unsigned *units_out) {
    static const char *const UNIT_KEYS[] = {"clock", "data", "writeEnable", "taps", NULL};
    Tap64Device *d = kind_alloc(ctx, sizeof(*d));
    const cJSON *units = kind_member(ctx, json, "units", 1);
    const cJSON *item;
    size_t n = 0;

    if (d == NULL || units == NULL) {
        return NULL;
    }
    if (!cJSON_IsArray(units) || cJSON_GetArraySize(units) < 1 || cJSON_GetArraySize(units) > 2) {
        kind_error(ctx, "\"units\" must be an array of 1 to 2 units");
        return NULL;
    }
    cJSON_ArrayForEach(item, units) {
        Tap64Unit *u = &d->units[n];

        kind_push(ctx, "units[%zu]", n);
        if (kind_check_keys(ctx, item, UNIT_KEYS, NULL) != 0 ||
            kind_string(ctx, item, "clock", &u->clock) != 0 ||
            kind_string(ctx, item, "data", &u->data) != 0 ||
            kind_string(ctx, item, "writeEnable", &u->we) != 0 ||
            seq_names(ctx, item, "taps", 4, 4, &u->taps, &u->tap_count) != 0) {
            return NULL;
        }
        kind_pop(ctx);
        n++;
    }
    d->unit_count = n;
    *units_out = (unsigned)n;
    return d;
}

static void *tap64_bind(KindContext *ctx, const void *raw, const PartPin *pins, size_t count) {
    const Tap64Device *d = raw;
    Tap64Bound *b = kind_alloc(ctx, sizeof(*b));
    size_t i;

    if (b == NULL) {
        return NULL;
    }
    for (i = 0; i < d->unit_count; i++) {
        const Tap64Unit *u = &d->units[i];
        Tap64Pins *p = &b->units[i];

        kind_push(ctx, "units[%zu]", i);
        if (seq_bind(ctx, pins, count, u->clock, &p->clock) != 0 ||
            seq_bind(ctx, pins, count, u->data, &p->data) != 0 ||
            seq_bind(ctx, pins, count, u->we, &p->we) != 0 ||
            bind_names(ctx, pins, count, u->taps, 4, p->taps) != 0) {
            return NULL;
        }
        kind_pop(ctx);
    }
    b->unit_count = d->unit_count;
    return b;
}

static ShiftHeader *tap64_create(const void *dev, const void *bound) {
    const Tap64Bound *b = bound;
    Tap64State *s = calloc(1, sizeof(*s));
    size_t i;

    (void)dev;
    if (s == NULL) {
        return NULL;
    }
    s->b = b;
    for (i = 0; i < b->unit_count; i++) {
        s->stages[i] = zero_bits(64);
        init_outputs(s->taps[i], b->units[i].taps, 4);
    }
    return &s->h;
}

static void tap64_eval(Sim *sim, SimInstance *inst, Tap64State *s, int at_start) {
    const Tap64Bound *b = s->b;
    size_t i;

    for (i = 0; i < b->unit_count; i++) {
        const Tap64Pins *p = &b->units[i];
        LogicValue clock = sim_read_pin(sim, inst, p->clock);
        int we = seq_read(sim, inst, p->we);
        unsigned k;

        if (!at_start && seq_rising(s->last_clock[i], clock)) {
            SeqBits shifted = shift_up(s->stages[i], 64, seq_read(sim, inst, p->data));
            SeqBits written = shifted;

            for (k = 0; k < 3; k++) {
                seq_set_bit(&written, 16 * (k + 1), seq_value(seq_read(sim, inst, p->taps[k])));
            }
            s->stages[i] = we == 0 ? shifted : we == 1 ? written : shift_merge(shifted, written);
        }
        s->last_clock[i] = clock;
        for (k = 0; k < 4; k++) {
            shift_drive(sim, inst, &s->taps[i][k],
                        seq_gate(seq_bit(s->stages[i], 16 * (k + 1) - 1), not3(we)),
                        SEQ_PUSH_PULL, s->h.timing);
        }
    }
}

static void tap64_start(Sim *sim, SimInstance *inst, ShiftHeader *h) {
    Tap64State *s = (Tap64State *)h;
    size_t i;

    for (i = 0; i < s->b->unit_count; i++) {
        s->last_clock[i] = sim_read_pin(sim, inst, s->b->units[i].clock);
    }
    tap64_eval(sim, inst, s, 1);
}

static void tap64_changed(Sim *sim, SimInstance *inst, ShiftHeader *h, size_t pin,
                          LogicValue old_value, LogicValue new_value) {
    (void)pin;
    (void)old_value;
    (void)new_value;
    tap64_eval(sim, inst, (Tap64State *)h, 0);
}

const ShiftModel TAP64_MODEL = {
    "tap64", TAP64_KEYS, NULL, tap64_load, tap64_bind, tap64_create, tap64_start, tap64_changed,
    NULL, NULL
};

/* ---- bus-register --------------------------------------------------------------- */

static const char *const BUSREG_KEYS[] = {
    "a", "b", "clock", "aEnable", "direction", "parallelSerial", "asyncSync", "serialIn", NULL
};

typedef struct {
    const char *const *a, *const *b;
    size_t a_count, b_count;
    const char *clock, *ae, *dir, *ps, *as, *si;
} BusRegDevice;

typedef struct {
    size_t a[8], b[8];
    size_t clock, ae, dir, ps, as, si;
} BusRegBound;

typedef struct {
    ShiftHeader h;
    const BusRegBound *b;
    SeqBits reg;
    LogicValue last_clock;
    SeqOutput a_out[8], b_out[8];
} BusRegState;

static void *busreg_load(KindContext *ctx, const cJSON *json, unsigned *units) {
    BusRegDevice *d = kind_alloc(ctx, sizeof(*d));

    if (d == NULL || seq_names(ctx, json, "a", 8, 8, &d->a, &d->a_count) != 0 ||
        seq_names(ctx, json, "b", 8, 8, &d->b, &d->b_count) != 0 ||
        kind_string(ctx, json, "clock", &d->clock) != 0 ||
        kind_string(ctx, json, "aEnable", &d->ae) != 0 ||
        kind_string(ctx, json, "direction", &d->dir) != 0 ||
        kind_string(ctx, json, "parallelSerial", &d->ps) != 0 ||
        kind_string(ctx, json, "asyncSync", &d->as) != 0 ||
        kind_string(ctx, json, "serialIn", &d->si) != 0) {
        return NULL;
    }
    *units = 1;
    return d;
}

static void *busreg_bind(KindContext *ctx, const void *raw, const PartPin *pins, size_t count) {
    const BusRegDevice *d = raw;
    BusRegBound *b = kind_alloc(ctx, sizeof(*b));

    if (b == NULL || bind_names(ctx, pins, count, d->a, 8, b->a) != 0 ||
        bind_names(ctx, pins, count, d->b, 8, b->b) != 0 ||
        seq_bind(ctx, pins, count, d->clock, &b->clock) != 0 ||
        seq_bind(ctx, pins, count, d->ae, &b->ae) != 0 ||
        seq_bind(ctx, pins, count, d->dir, &b->dir) != 0 ||
        seq_bind(ctx, pins, count, d->ps, &b->ps) != 0 ||
        seq_bind(ctx, pins, count, d->as, &b->as) != 0 ||
        seq_bind(ctx, pins, count, d->si, &b->si) != 0) {
        return NULL;
    }
    return b;
}

static ShiftHeader *busreg_create(const void *dev, const void *bound) {
    BusRegState *s = calloc(1, sizeof(*s));
    const BusRegBound *b = bound;

    (void)dev;
    if (s == NULL) {
        return NULL;
    }
    s->b = b;
    s->reg = zero_bits(8);
    init_outputs(s->a_out, b->a, 8);
    init_outputs(s->b_out, b->b, 8);
    return &s->h;
}

/* The input bus for A/B = dir_level and AE = ae_level (both known). */
static SeqBits busreg_input(const Sim *sim, const SimInstance *inst, const BusRegState *s, int dir,
                            int ae) {
    if (dir) {
        return ae ? shift_read_bits(sim, inst, s->b->a, 8) : s->reg;
    }
    return shift_read_bits(sim, inst, s->b->b, 8);
}

static void busreg_eval(Sim *sim, SimInstance *inst, BusRegState *s, int at_start) {
    const BusRegBound *b = s->b;
    LogicValue clock = sim_read_pin(sim, inst, b->clock);
    int edge = !at_start && seq_rising(s->last_clock, clock);
    int ae = seq_read(sim, inst, b->ae);
    int dir = seq_read(sim, inst, b->dir);
    int ps = seq_read(sim, inst, b->ps);
    int as = seq_read(sim, inst, b->as);
    SeqBits r = {0, 0};
    int first = 1;
    int p, a, di, e;

    s->last_clock = clock;
    for (p = lo(ps); p <= hi(ps); p++) {
        for (a = lo(as); a <= hi(as); a++) {
            for (di = lo(dir); di <= hi(dir); di++) {
                for (e = lo(ae); e <= hi(ae); e++) {
                    SeqBits o = s->reg;

                    if (!p) {
                        if (edge) {
                            o = shift_up(s->reg, 8, seq_read(sim, inst, b->si));
                        }
                    } else if (a || edge) {
                        o = busreg_input(sim, inst, s, di, e);
                    }
                    r = first ? o : shift_merge(r, o);
                    first = 0;
                }
            }
        }
    }
    s->reg = r;
    drive_bits(sim, inst, s->b_out, 8, s->reg, dir, s->h.timing);
    drive_bits(sim, inst, s->a_out, 8, s->reg, and3(not3(dir), ae), s->h.timing);
}

static void busreg_start(Sim *sim, SimInstance *inst, ShiftHeader *h) {
    BusRegState *s = (BusRegState *)h;

    s->last_clock = sim_read_pin(sim, inst, s->b->clock);
    busreg_eval(sim, inst, s, 1);
}

static void busreg_changed(Sim *sim, SimInstance *inst, ShiftHeader *h, size_t pin,
                           LogicValue old_value, LogicValue new_value) {
    (void)pin;
    (void)old_value;
    (void)new_value;
    busreg_eval(sim, inst, (BusRegState *)h, 0);
}

const ShiftModel BUSREG_MODEL = {
    "bus-register", BUSREG_KEYS, NULL, busreg_load, busreg_bind, busreg_create, busreg_start,
    busreg_changed, NULL, NULL
};

/* ---- fifo ----------------------------------------------------------------------- */

#define FIFO_WORDS 16

static const char *const FIFO_KEYS[] = {
    "data", "outputs", "shiftIn", "shiftOut", "inputReady", "outputReady", "reset",
    "outputEnable", "words", "unloadNeedsEnable", "loadWhileHigh", NULL
};
static const ShiftExtraKey FIFO_EXTRA[] = {{"fallThrough", 1}, {NULL, 0}};

typedef struct {
    const char *const *data, *const *outputs;
    size_t data_count, output_count;
    const char *si, *so, *dir, *dor;
    SeqControlName reset, oe;
    int unload_needs_enable, load_while_high;
} FifoDevice;

typedef struct {
    const FifoDevice *dev;
    size_t data[4], outputs[4];
    size_t si, so, dir, dor;
    SeqControl reset, oe;
} FifoBound;

typedef struct {
    ShiftHeader h;
    const FifoBound *b;
    int full[FIFO_WORDS];
    SeqBits word[FIFO_WORDS];
    SeqBits shown;           /* what Q shows: position 16's word, or the last one */
    LogicValue last_si, last_strobe;
    int timer_pending;
    SeqOutput q[4], dir, dor;
} FifoState;

static void *fifo_load(KindContext *ctx, const cJSON *json, unsigned *units) {
    FifoDevice *d = kind_alloc(ctx, sizeof(*d));
    uint64_t words;

    if (d == NULL || seq_names(ctx, json, "data", 4, 4, &d->data, &d->data_count) != 0 ||
        seq_names(ctx, json, "outputs", 4, 4, &d->outputs, &d->output_count) != 0 ||
        kind_string(ctx, json, "shiftIn", &d->si) != 0 ||
        kind_string(ctx, json, "shiftOut", &d->so) != 0 ||
        kind_string(ctx, json, "inputReady", &d->dir) != 0 ||
        kind_string(ctx, json, "outputReady", &d->dor) != 0 ||
        seq_control_name(ctx, json, "reset", 1, 0, &d->reset) != 0 ||
        seq_control_name(ctx, json, "outputEnable", 1, 0, &d->oe) != 0 ||
        kind_uint(ctx, json, "words", FIFO_WORDS, &words) != 0 ||
        kind_bool(ctx, json, "unloadNeedsEnable", &d->unload_needs_enable) != 0 ||
        kind_bool(ctx, json, "loadWhileHigh", &d->load_while_high) != 0) {
        return NULL;
    }
    if (words != FIFO_WORDS) {
        kind_error(ctx, "\"words\" must be %d", FIFO_WORDS);
        return NULL;
    }
    *units = 1;
    return d;
}

static void *fifo_bind(KindContext *ctx, const void *raw, const PartPin *pins, size_t count) {
    const FifoDevice *d = raw;
    FifoBound *b = kind_alloc(ctx, sizeof(*b));

    if (b == NULL || bind_names(ctx, pins, count, d->data, 4, b->data) != 0 ||
        bind_names(ctx, pins, count, d->outputs, 4, b->outputs) != 0 ||
        seq_bind(ctx, pins, count, d->si, &b->si) != 0 ||
        seq_bind(ctx, pins, count, d->so, &b->so) != 0 ||
        seq_bind(ctx, pins, count, d->dir, &b->dir) != 0 ||
        seq_bind(ctx, pins, count, d->dor, &b->dor) != 0 ||
        seq_bind_control(ctx, pins, count, &d->reset, &b->reset) != 0 ||
        seq_bind_control(ctx, pins, count, &d->oe, &b->oe) != 0) {
        return NULL;
    }
    b->dev = d;
    return b;
}

static ShiftHeader *fifo_create(const void *dev, const void *bound) {
    const FifoBound *b = bound;
    FifoState *s = calloc(1, sizeof(*s));
    int i;

    (void)dev;
    if (s == NULL) {
        return NULL;
    }
    s->b = b;
    for (i = 0; i < FIFO_WORDS; i++) {
        s->word[i] = zero_bits(4);
    }
    s->shown = zero_bits(4);
    init_outputs(s->q, b->outputs, 4);
    init_outputs(&s->dir, &b->dir, 1);
    init_outputs(&s->dor, &b->dor, 1);
    return &s->h;
}

static SimTime fifo_step(const FifoState *s) {
    SimTime step = s->h.timing->extra[0] / (FIFO_WORDS - 1);

    return step > 0 ? step : 1;
}

static int fifo_can_move(const FifoState *s) {
    int i;

    for (i = 0; i < FIFO_WORDS - 1; i++) {
        if (s->full[i] && !s->full[i + 1]) {
            return 1;
        }
    }
    return 0;
}

static void fifo_outputs(Sim *sim, SimInstance *inst, FifoState *s) {
    const FifoBound *b = s->b;
    const ShiftTiming *t = s->h.timing;

    if (s->full[FIFO_WORDS - 1]) {
        s->shown = s->word[FIFO_WORDS - 1];
    }
    shift_drive(sim, inst, &s->dir, s->full[0] ? LOGIC_0 : LOGIC_1, SEQ_PUSH_PULL, t);
    shift_drive(sim, inst, &s->dor, s->full[FIFO_WORDS - 1] ? LOGIC_1 : LOGIC_0, SEQ_PUSH_PULL, t);
    drive_bits(sim, inst, s->q, 4, s->shown, seq_enable(sim, inst, &b->oe), t);
}

static void fifo_schedule(Sim *sim, SimInstance *inst, FifoState *s) {
    if (!s->timer_pending && fifo_can_move(s)) {
        s->timer_pending = 1;
        sim_schedule_timer(sim, inst, 1, fifo_step(s));
    }
}

static void fifo_try_load(Sim *sim, SimInstance *inst, FifoState *s, int edge) {
    const FifoBound *b = s->b;
    int si = seq_read(sim, inst, b->si);

    if (s->full[0] || seq_async(sim, inst, &b->reset) != 0) {
        return;
    }
    if (edge || (b->dev->load_while_high && si == 1)) {
        s->word[0] = shift_read_bits(sim, inst, b->data, 4);
        s->full[0] = 1;
    }
}

static void fifo_eval(Sim *sim, SimInstance *inst, FifoState *s, int at_start) {
    const FifoBound *b = s->b;
    LogicValue si = sim_read_pin(sim, inst, b->si);
    int reset = seq_async(sim, inst, &b->reset);
    int strobe_level = seq_read(sim, inst, b->so);
    LogicValue strobe;
    int i;

    if (b->dev->unload_needs_enable) {
        strobe_level = or3(strobe_level, not3(seq_enable(sim, inst, &b->oe)));
    }
    strobe = seq_value(strobe_level);
    if (reset != 0) {
        /* MR (an unknown MR is taken as a reset): every position empty, data kept. */
        for (i = 0; i < FIFO_WORDS; i++) {
            s->full[i] = 0;
        }
    } else if (!at_start) {
        if (seq_falling(s->last_strobe, strobe) && s->full[FIFO_WORDS - 1]) {
            s->full[FIFO_WORDS - 1] = 0;
        }
        fifo_try_load(sim, inst, s, seq_rising(s->last_si, si));
    }
    s->last_si = si;
    s->last_strobe = strobe;
    fifo_schedule(sim, inst, s);
    fifo_outputs(sim, inst, s);
}

static void fifo_timer(Sim *sim, SimInstance *inst, ShiftHeader *h, uint32_t id) {
    FifoState *s = (FifoState *)h;
    int i;

    (void)id;
    s->timer_pending = 0;
    /* One move per position per step, from the output end, so words ripple forward. */
    for (i = FIFO_WORDS - 2; i >= 0; i--) {
        if (s->full[i] && !s->full[i + 1]) {
            s->word[i + 1] = s->word[i];
            s->full[i + 1] = 1;
            s->full[i] = 0;
            i--;   /* the word just moved does not move again this step */
        }
    }
    fifo_try_load(sim, inst, s, 0);
    fifo_schedule(sim, inst, s);
    fifo_outputs(sim, inst, s);
}

static void fifo_start(Sim *sim, SimInstance *inst, ShiftHeader *h) {
    FifoState *s = (FifoState *)h;
    int strobe_level = seq_read(sim, inst, s->b->so);

    if (s->b->dev->unload_needs_enable) {
        strobe_level = or3(strobe_level, not3(seq_enable(sim, inst, &s->b->oe)));
    }
    s->last_si = sim_read_pin(sim, inst, s->b->si);
    s->last_strobe = seq_value(strobe_level);
    fifo_eval(sim, inst, s, 1);
}

static void fifo_changed(Sim *sim, SimInstance *inst, ShiftHeader *h, size_t pin,
                         LogicValue old_value, LogicValue new_value) {
    (void)pin;
    (void)old_value;
    (void)new_value;
    fifo_eval(sim, inst, (FifoState *)h, 0);
}

const ShiftModel FIFO_MODEL = {
    "fifo", FIFO_KEYS, FIFO_EXTRA, fifo_load, fifo_bind, fifo_create, fifo_start, fifo_changed,
    fifo_timer, NULL
};

/* ---- serial-in (SN74LV8153) ---------------------------------------------------- */

static const char *const SERIN_KEYS[] = {
    "data", "address", "outputs", "outputSelect", "reset", "outputEnable", "serialOut", NULL
};

enum { RX_IDLE, RX_START, RX_BITS };
enum { TIMER_SAMPLE = 1, TIMER_SOUT = 2 };

typedef struct {
    const char *data, *outsel, *sout;
    const char *const *address, *const *outputs;
    size_t address_count, output_count;
    SeqControlName reset, oe;
} SerInDevice;

typedef struct {
    size_t data, outsel, sout;
    size_t address[3], outputs[8];
    SeqControl reset, oe;
} SerInBound;

typedef struct {
    ShiftHeader h;
    const SerInBound *b;
    int state;
    SimTime t0;
    uint32_t gen;            /* frame generation: stale timers are ignored */
    int bits[9];             /* sampled levels, k = 2..8 */
    int second;              /* the next good frame completes a byte */
    int first_addr;
    int first_nibble[4];
    SeqBits reg;
    LogicValue last_data;
    SeqOutput y[8], sout;
} SerInState;

static void *serin_load(KindContext *ctx, const cJSON *json, unsigned *units) {
    SerInDevice *d = kind_alloc(ctx, sizeof(*d));

    if (d == NULL || kind_string(ctx, json, "data", &d->data) != 0 ||
        seq_names(ctx, json, "address", 3, 3, &d->address, &d->address_count) != 0 ||
        seq_names(ctx, json, "outputs", 8, 8, &d->outputs, &d->output_count) != 0 ||
        kind_string(ctx, json, "outputSelect", &d->outsel) != 0 ||
        seq_control_name(ctx, json, "reset", 1, 0, &d->reset) != 0 ||
        seq_control_name(ctx, json, "outputEnable", 1, 0, &d->oe) != 0 ||
        kind_string(ctx, json, "serialOut", &d->sout) != 0) {
        return NULL;
    }
    *units = 1;
    return d;
}

static void *serin_bind(KindContext *ctx, const void *raw, const PartPin *pins, size_t count) {
    const SerInDevice *d = raw;
    SerInBound *b = kind_alloc(ctx, sizeof(*b));

    if (b == NULL || seq_bind(ctx, pins, count, d->data, &b->data) != 0 ||
        seq_bind(ctx, pins, count, d->outsel, &b->outsel) != 0 ||
        seq_bind(ctx, pins, count, d->sout, &b->sout) != 0 ||
        bind_names(ctx, pins, count, d->address, 3, b->address) != 0 ||
        bind_names(ctx, pins, count, d->outputs, 8, b->outputs) != 0 ||
        seq_bind_control(ctx, pins, count, &d->reset, &b->reset) != 0 ||
        seq_bind_control(ctx, pins, count, &d->oe, &b->oe) != 0) {
        return NULL;
    }
    return b;
}

static ShiftHeader *serin_create(const void *dev, const void *bound) {
    const SerInBound *b = bound;
    SerInState *s = calloc(1, sizeof(*s));

    (void)dev;
    if (s == NULL) {
        return NULL;
    }
    s->b = b;
    s->reg = zero_bits(8);
    init_outputs(s->y, b->outputs, 8);
    init_outputs(&s->sout, &b->sout, 1);
    return &s->h;
}

static void serin_outputs(Sim *sim, SimInstance *inst, SerInState *s) {
    const SerInBound *b = s->b;
    int enable = seq_enable(sim, inst, &b->oe);
    int pp = seq_read(sim, inst, b->outsel);
    unsigned i;

    for (i = 0; i < 8; i++) {
        LogicValue bitv = seq_bit(s->reg, i);
        LogicValue v;

        if (pp == 1) {
            v = bitv;
        } else if (pp == 0) {
            v = bitv == LOGIC_1 ? LOGIC_0 : bitv == LOGIC_0 ? LOGIC_Z : LOGIC_X;
        } else {
            v = LOGIC_X;
        }
        shift_drive(sim, inst, &s->y[i], seq_gate(v, enable), SEQ_PUSH_PULL, s->h.timing);
    }
}

static void serin_reset(SerInState *s) {
    s->reg = zero_bits(8);
    s->state = RX_IDLE;
    s->second = 0;
    s->gen++;
}

/* The frame's last sample: check the address, then store a nibble or complete the byte. */
static void serin_frame(Sim *sim, SimInstance *inst, SerInState *s, SimTime period) {
    const SerInBound *b = s->b;
    int addr = 0;
    int known = 1;
    int i;

    s->state = RX_IDLE;
    for (i = 0; i < 3; i++) {
        int pin = seq_read(sim, inst, b->address[i]);

        if (s->bits[2 + i] < 0 || pin < 0) {
            known = 0;
        } else if (s->bits[2 + i] != pin) {
            return;   /* another device's frame: ignored */
        }
        addr |= (s->bits[2 + i] > 0 ? 1 : 0) << i;
    }
    if (!known) {
        return;
    }
    if (!s->second) {
        s->first_addr = addr;
        for (i = 0; i < 4; i++) {
            s->first_nibble[i] = s->bits[5 + i];
        }
        s->second = 1;
        return;
    }
    s->second = 0;
    if (addr != s->first_addr) {
        return;   /* the pair's addresses differ: dropped */
    }
    for (i = 0; i < 4; i++) {
        seq_set_bit(&s->reg, (unsigned)i, seq_value(s->first_nibble[i]));
        seq_set_bit(&s->reg, (unsigned)(4 + i), seq_value(s->bits[5 + i]));
    }
    shift_drive(sim, inst, &s->sout, LOGIC_0, SEQ_PUSH_PULL, s->h.timing);
    sim_schedule_timer(sim, inst, (TIMER_SOUT << 24) | (s->gen & 0xFFFFu), period);
    serin_outputs(sim, inst, s);
}

static void serin_eval(Sim *sim, SimInstance *inst, SerInState *s, int at_start) {
    const SerInBound *b = s->b;
    LogicValue data = sim_read_pin(sim, inst, b->data);
    int reset = seq_async(sim, inst, &b->reset);
    SimTime now = sim_now(sim);

    if (reset != 0) {
        serin_reset(s);
        if (reset < 0) {
            s->reg.known = 0;
        }
    } else if (!at_start) {
        if (s->state == RX_IDLE && seq_falling(s->last_data, data)) {
            s->state = RX_START;
            s->t0 = now;
        } else if (s->state == RX_START && seq_rising(s->last_data, data)) {
            SimTime period = now - s->t0;
            int k;

            s->state = RX_BITS;
            s->gen++;
            for (k = 2; k <= 8; k++) {
                /* sample bit k at t0 + (k + 1/2) P, that is (k - 1/2) P from now */
                sim_schedule_timer(sim, inst,
                                   (TIMER_SAMPLE << 24) | ((s->gen & 0xFFFFu) << 8) | (uint32_t)k,
                                   (SimTime)k * period - period / 2);
            }
        }
    }
    s->last_data = data;
    if (at_start) {
        shift_drive(sim, inst, &s->sout, LOGIC_1, SEQ_PUSH_PULL, s->h.timing);
    }
    serin_outputs(sim, inst, s);
}

static void serin_timer(Sim *sim, SimInstance *inst, ShiftHeader *h, uint32_t id) {
    SerInState *s = (SerInState *)h;
    uint32_t kind = id >> 24;

    if (kind == TIMER_SOUT) {
        if ((id & 0xFFFFu) == (s->gen & 0xFFFFu)) {
            shift_drive(sim, inst, &s->sout, LOGIC_1, SEQ_PUSH_PULL, s->h.timing);
        }
        return;
    }
    if (((id >> 8) & 0xFFFFu) != (s->gen & 0xFFFFu) || s->state != RX_BITS) {
        return;   /* a timer of a frame that a reset cancelled */
    }
    {
        int k = (int)(id & 0xFFu);

        s->bits[k] = seq_read(sim, inst, s->b->data);
        if (k == 8) {
            SimTime period = (sim_now(sim) - s->t0) * 2 / 17;   /* now = t0 + 8.5 P */

            serin_frame(sim, inst, s, period);
        }
    }
}

static void serin_start(Sim *sim, SimInstance *inst, ShiftHeader *h) {
    SerInState *s = (SerInState *)h;

    s->last_data = sim_read_pin(sim, inst, s->b->data);
    serin_eval(sim, inst, s, 1);
}

static void serin_changed(Sim *sim, SimInstance *inst, ShiftHeader *h, size_t pin,
                          LogicValue old_value, LogicValue new_value) {
    (void)pin;
    (void)old_value;
    (void)new_value;
    serin_eval(sim, inst, (SerInState *)h, 0);
}

const ShiftModel SERIN_MODEL = {
    "serial-in", SERIN_KEYS, NULL, serin_load, serin_bind, serin_create, serin_start,
    serin_changed, serin_timer, NULL
};
