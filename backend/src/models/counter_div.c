#include "counter_div_int.h"
#include "logic_alloc.h"

static int lo(int level) {
    return level < 0 ? 0 : level;
}

static int hi(int level) {
    return level < 0 ? 1 : level;
}

int cdv_and3(int a, int b) {
    if (a == 0 || b == 0) {
        return 0;
    }
    return a == 1 && b == 1 ? 1 : -1;
}

int cdv_or3(int a, int b) {
    if (a == 1 || b == 1) {
        return 1;
    }
    return a == 0 && b == 0 ? 0 : -1;
}

int cdv_not3(int a) {
    return a < 0 ? -1 : !a;
}

int cdv_bind_names(KindContext *ctx, const PartPin *pins, size_t count,
                      const char *const *names, size_t n, size_t *out) {
    size_t i;

    for (i = 0; i < n; i++) {
        if (seq_bind(ctx, pins, count, names[i], &out[i]) != 0) {
            return -1;
        }
    }
    return 0;
}

void cdv_init_outputs(SeqOutput *outputs, const size_t *pins, size_t n) {
    size_t i;

    for (i = 0; i < n; i++) {
        outputs[i].pin = pins[i];
        outputs[i].scheduled = 0;
        outputs[i].driven = LOGIC_X;
    }
}

void cdv_drive(Sim *sim, SimInstance *inst, SeqOutput *o, int level, const CounterTiming *t) {
    counter_drive(sim, inst, o, seq_value(level), t, 0);
}

/* An optional {"in", "out"} pair of pin names. */
int cdv_pin_pair(KindContext *ctx, const cJSON *json, const char *key, const char **in,
                    const char **out) {
    static const char *const KEYS[] = {"in", "out", NULL};
    const cJSON *item = cJSON_GetObjectItemCaseSensitive(json, key);

    *in = NULL;
    *out = NULL;
    if (item == NULL) {
        return 0;
    }
    kind_push(ctx, "%s", key);
    if (kind_check_keys(ctx, item, KEYS, NULL) != 0 || kind_string(ctx, item, "in", in) != 0 ||
        kind_string(ctx, item, "out", out) != 0) {
        return -1;
    }
    kind_pop(ctx);
    return 0;
}

static const char *const JOHNSON_KEYS[] = {
    "stages", "clock", "clockGate", "reset", "outputs", "carry", "data", "presetEnable", "jam",
    "qbar", NULL
};

static void *johnson_load(KindContext *ctx, const cJSON *json, unsigned *units) {
    JohnsonDevice *dev = kind_alloc(ctx, sizeof(*dev));
    uint64_t stages;

    if (dev == NULL || kind_uint(ctx, json, "stages", JOHNSON_MAX, &stages) != 0) {
        return NULL;
    }
    if (stages < 4) {
        kind_error(ctx, "\"stages\" must be 4 or 5");
        return NULL;
    }
    dev->stages = (unsigned)stages;
    if (kind_string(ctx, json, "clock", &dev->clock) != 0 ||
        seq_gate_name(ctx, json, "clockGate", &dev->gate) != 0 ||
        seq_control_name(ctx, json, "reset", 1, 0, &dev->reset) != 0 ||
        seq_control_name(ctx, json, "presetEnable", 0, 0, &dev->preset) != 0 ||
        seq_optional_name(ctx, json, "carry", &dev->carry) != 0 ||
        seq_optional_name(ctx, json, "data", &dev->data) != 0) {
        return NULL;
    }
    if (cJSON_GetObjectItemCaseSensitive(json, "outputs") != NULL &&
        seq_names(ctx, json, "outputs", 2 * dev->stages, 2 * dev->stages, &dev->outputs,
                  &dev->output_count) != 0) {
        return NULL;
    }
    if (cJSON_GetObjectItemCaseSensitive(json, "qbar") != NULL &&
        seq_names(ctx, json, "qbar", dev->stages, dev->stages, &dev->qbar, &dev->qbar_count) !=
            0) {
        return NULL;
    }
    if (dev->preset.pin != NULL) {
        if (seq_names(ctx, json, "jam", dev->stages, dev->stages, &dev->jam, &dev->jam_count) !=
            0) {
            return NULL;
        }
    } else if (cJSON_GetObjectItemCaseSensitive(json, "jam") != NULL) {
        kind_error(ctx, "\"jam\" needs \"presetEnable\"");
        return NULL;
    }
    *units = 1;
    return dev;
}

static void *johnson_bind(KindContext *ctx, const void *raw, const PartPin *pins, size_t count) {
    const JohnsonDevice *dev = raw;
    JohnsonBound *b = kind_alloc(ctx, sizeof(*b));

    if (b == NULL || seq_bind(ctx, pins, count, dev->clock, &b->clock) != 0 ||
        seq_bind_gate(ctx, pins, count, &dev->gate, &b->gate) != 0 ||
        seq_bind_control(ctx, pins, count, &dev->reset, &b->reset) != 0 ||
        seq_bind_control(ctx, pins, count, &dev->preset, &b->preset) != 0 ||
        cdv_bind_names(ctx, pins, count, dev->outputs, dev->output_count, b->outputs) != 0 ||
        seq_bind(ctx, pins, count, dev->carry, &b->carry) != 0 ||
        seq_bind(ctx, pins, count, dev->data, &b->data) != 0 ||
        cdv_bind_names(ctx, pins, count, dev->jam, dev->jam_count, b->jam) != 0 ||
        cdv_bind_names(ctx, pins, count, dev->qbar, dev->qbar_count, b->qbar) != 0) {
        return NULL;
    }
    b->dev = dev;
    return b;
}

static CounterHeader *johnson_create(const void *dev, const void *bound) {
    const JohnsonBound *b = bound;
    JohnsonState *s = logic_calloc(1, sizeof(*s));

    (void)dev;
    if (s == NULL) {
        return NULL;
    }
    s->b = b;
    cdv_init_outputs(s->outputs, b->outputs, b->dev->output_count);
    cdv_init_outputs(&s->carry, &b->carry, 1);
    cdv_init_outputs(s->qbar, b->qbar, b->dev->qbar_count);
    return &s->h;
}

static LogicValue johnson_clock(const Sim *sim, const SimInstance *inst, const JohnsonBound *b) {
    LogicValue clock = sim_read_pin(sim, inst, b->clock);

    if (b->gate.pin != SEQ_NO_PIN) {
        clock = seq_gated_clock(clock, sim_read_pin(sim, inst, b->gate.pin), b->gate.op);
    }
    return clock;
}

static void johnson_eval(Sim *sim, SimInstance *inst, JohnsonState *s, int at_start) {
    const JohnsonBound *b = s->b;
    const JohnsonDevice *dev = b->dev;
    unsigned n = dev->stages;
    LogicValue clock = johnson_clock(sim, inst, b);
    int edge = !at_start && seq_rising(s->last_clock, clock);
    int reset = seq_async(sim, inst, &b->reset);
    int preset = seq_async(sim, inst, &b->preset);
    int next[JOHNSON_MAX];
    int result[JOHNSON_MAX];
    unsigned k;
    int r, p;

    s->last_clock = clock;
    for (k = 0; k < n; k++) {
        next[k] = s->q[k];
    }
    if (edge) {
        next[0] = b->data != SEQ_NO_PIN ? seq_read(sim, inst, b->data) : cdv_not3(s->q[n - 1]);
        next[1] = s->q[0];
        next[2] = cdv_and3(s->q[1], cdv_or3(s->q[0], s->q[2]));
        for (k = 3; k < n; k++) {
            next[k] = s->q[k - 1];
        }
    }
    for (k = 0; k < n; k++) {
        int have = 0;
        int v = -1;

        for (r = lo(reset); r <= hi(reset); r++) {
            for (p = lo(preset); p <= hi(preset); p++) {
                int o = r ? 0 : p ? seq_read(sim, inst, b->jam[k]) : next[k];

                v = !have ? o : v == o ? v : -1;
                have = 1;
            }
        }
        result[k] = v;
    }
    for (k = 0; k < n; k++) {
        s->q[k] = result[k];
    }
    /* Decoded outputs (Q1 = q[0], QS = q[n-1]). */
    if (dev->output_count > 0) {
        const int *q = s->q;

        cdv_drive(sim, inst, &s->outputs[0], cdv_and3(cdv_not3(q[0]), cdv_not3(q[n - 1])), s->h.timing);
        for (k = 1; k < n; k++) {
            cdv_drive(sim, inst, &s->outputs[k], cdv_and3(q[k - 1], cdv_not3(q[k])), s->h.timing);
        }
        cdv_drive(sim, inst, &s->outputs[n], cdv_and3(q[0], q[n - 1]), s->h.timing);
        for (k = 1; k < n; k++) {
            cdv_drive(sim, inst, &s->outputs[n + k], cdv_and3(cdv_not3(q[k - 1]), q[k]), s->h.timing);
        }
    }
    cdv_drive(sim, inst, &s->carry, cdv_not3(s->q[n - 1]), s->h.timing);
    for (k = 0; k < dev->qbar_count; k++) {
        cdv_drive(sim, inst, &s->qbar[k], cdv_not3(s->q[k]), s->h.timing);
    }
}

static void johnson_start(Sim *sim, SimInstance *inst, CounterHeader *h) {
    JohnsonState *s = (JohnsonState *)h;

    s->last_clock = johnson_clock(sim, inst, s->b);
    johnson_eval(sim, inst, s, 1);
}

static void johnson_changed(Sim *sim, SimInstance *inst, CounterHeader *h, size_t pin,
                            LogicValue old_value, LogicValue new_value) {
    (void)pin;
    (void)old_value;
    (void)new_value;
    johnson_eval(sim, inst, (JohnsonState *)h, 0);
}

const CounterModel JOHNSON_MODEL = {
    "johnson", JOHNSON_KEYS, NULL, johnson_load, johnson_bind, johnson_create, NULL,
    johnson_start, johnson_changed, NULL
};

/* ---- divn ---------------------------------------------------------------------- */

static const char *const DIVN_KEYS[] = {"clock", "jam", "modes", "latchEnable", "output", NULL};

static void *divn_load(KindContext *ctx, const cJSON *json, unsigned *units) {
    DivnDevice *dev = kind_alloc(ctx, sizeof(*dev));

    if (dev == NULL || kind_string(ctx, json, "clock", &dev->clock) != 0 ||
        seq_names(ctx, json, "jam", 16, 16, &dev->jam, &dev->jam_count) != 0 ||
        seq_names(ctx, json, "modes", 3, 3, &dev->modes, &dev->mode_count) != 0 ||
        kind_string(ctx, json, "latchEnable", &dev->latch) != 0 ||
        kind_string(ctx, json, "output", &dev->output) != 0) {
        return NULL;
    }
    *units = 1;
    return dev;
}

static void *divn_bind(KindContext *ctx, const void *raw, const PartPin *pins, size_t count) {
    const DivnDevice *d = raw;
    DivnBound *b = kind_alloc(ctx, sizeof(*b));

    if (b == NULL || seq_bind(ctx, pins, count, d->clock, &b->clock) != 0 ||
        seq_bind(ctx, pins, count, d->latch, &b->latch) != 0 ||
        seq_bind(ctx, pins, count, d->output, &b->output) != 0 ||
        cdv_bind_names(ctx, pins, count, d->jam, 16, b->jam) != 0 ||
        cdv_bind_names(ctx, pins, count, d->modes, 3, b->modes) != 0) {
        return NULL;
    }
    return b;
}

static CounterHeader *divn_create(const void *dev, const void *bound) {
    const DivnBound *b = bound;
    DivnState *s = logic_calloc(1, sizeof(*s));

    (void)dev;
    if (s == NULL) {
        return NULL;
    }
    s->b = b;
    cdv_init_outputs(&s->out, &b->output, 1);
    return &s->h;
}

/* Jam pins first..first+n-1 as a binary number, or -1. */
static long jam_value(const Sim *sim, const SimInstance *inst, const DivnBound *b, unsigned first,
                      unsigned n) {
    long v = 0;
    unsigned i;

    for (i = 0; i < n; i++) {
        int level = seq_read(sim, inst, b->jam[first + i]);

        if (level < 0) {
            return -1;
        }
        v |= (long)level << i;
    }
    return v;
}

/* Mode: 2, 4, 5, 8, 10; 0 = master preset; -1 unknown or unlisted. */
static int divn_mode(const Sim *sim, const SimInstance *inst, const DivnBound *b) {
    int ka = seq_read(sim, inst, b->modes[0]);
    int kb = seq_read(sim, inst, b->modes[1]);
    int kc = seq_read(sim, inst, b->modes[2]);

    if (kb == 0 && kc == 0) {
        return 0;
    }
    if (ka < 0 || kb < 0 || kc < 0) {
        return -1;
    }
    switch (ka * 4 + kb * 2 + kc) {
        case 7: return 2;    /* 111 */
        case 3: return 4;    /* 011 */
        case 5: return 5;    /* 101 */
        case 1: return 8;    /* 001 */
        case 6: return 10;   /* 110 */
        default: return -1;  /* 010: not in Table I */
    }
}

/* N for a counting mode (2, 4, 5, 8, 10), or -1 when a jam pin is unknown. */
static long divn_n(const Sim *sim, const SimInstance *inst, const DivnBound *b, int mode) {
    unsigned d1_bits, d5_bits;
    long d1, d5, d2, d3, d4, n;

    switch (mode) {
        case 2: d1_bits = 1; break;
        case 4: d1_bits = 2; break;
        case 5: case 8: d1_bits = 3; break;
        case 10: d1_bits = 4; break;
        default: return -1;
    }
    d5_bits = 4 - d1_bits;
    d1 = jam_value(sim, inst, b, 0, d1_bits);
    d5 = d5_bits > 0 ? jam_value(sim, inst, b, d1_bits, d5_bits) : 0;
    d2 = jam_value(sim, inst, b, 4, 4);
    d3 = jam_value(sim, inst, b, 8, 4);
    d4 = jam_value(sim, inst, b, 12, 4);
    if (d1 < 0 || d5 < 0 || d2 < 0 || d3 < 0 || d4 < 0) {
        return -1;
    }
    n = mode * (1000 * d5 + 100 * d4 + 10 * d3 + d2) + d1;
    return n < 3 ? 3 : n;
}

static void divn_output(Sim *sim, SimInstance *inst, DivnState *s) {
    int le = seq_read(sim, inst, s->b->latch);
    int level;

    if (le == 0) {
        s->latched = 0;
    } else if (le == 1 && s->pulse == 1) {
        s->latched = 1;   /* latch enable high during (or at) a pulse */
    }
    level = cdv_or3(s->pulse, cdv_and3(s->latched ? 1 : 0, le));
    cdv_drive(sim, inst, &s->out, level, s->h.timing);
}

static void divn_eval(Sim *sim, SimInstance *inst, DivnState *s, int at_start) {
    const DivnBound *b = s->b;
    LogicValue clock = sim_read_pin(sim, inst, b->clock);
    int edge = !at_start && seq_rising(s->last_clock, clock);

    s->last_clock = clock;
    if (edge) {
        int mode = divn_mode(sim, inst, b);

        if (mode == 0) {
            /* Master preset: every section jammed and held. C takes N of the
               counting mode at the extra count that follows the release. */
            s->count = 0;
            s->preset = 1;
            s->pulse = 0;
        } else if (mode < 0 || s->count < 0) {
            s->count = -1;
            s->pulse = -1;
        } else if (s->preset) {
            s->preset = 0;
            s->count = divn_n(sim, inst, b, mode);
            s->pulse = s->count < 0 ? -1 : 0;
        } else if (s->count <= 1) {
            s->count = divn_n(sim, inst, b, mode);
            s->pulse = s->count < 0 ? -1 : 1;
        } else {
            s->count--;
            s->pulse = 0;
        }
    }
    if (at_start) {
        int mode = divn_mode(sim, inst, b);

        s->count = mode > 0 ? divn_n(sim, inst, b, mode) : -1;
        s->preset = 0;
        s->pulse = s->count < 0 ? -1 : 0;
    }
    divn_output(sim, inst, s);
}

static void divn_start(Sim *sim, SimInstance *inst, CounterHeader *h) {
    DivnState *s = (DivnState *)h;

    s->last_clock = sim_read_pin(sim, inst, s->b->clock);
    divn_eval(sim, inst, s, 1);
}

static void divn_changed(Sim *sim, SimInstance *inst, CounterHeader *h, size_t pin,
                         LogicValue old_value, LogicValue new_value) {
    (void)pin;
    (void)old_value;
    (void)new_value;
    divn_eval(sim, inst, (DivnState *)h, 0);
}

const CounterModel DIVN_MODEL = {
    "divn", DIVN_KEYS, NULL, divn_load, divn_bind, divn_create, NULL, divn_start, divn_changed,
    NULL
};

/* ---- rate --------------------------------------------------------------------- */

static const char *const RATE_KEYS[] = {
    "variant", "clock", "rate", "enable", "strobe", "cascade", "clear", "set", "out", "outBar",
    "terminal", "enableOut", NULL
};

static void *rate_load(KindContext *ctx, const cJSON *json, unsigned *units) {
    RateDevice *dev = kind_alloc(ctx, sizeof(*dev));
    const char *variant;

    if (dev == NULL || kind_string(ctx, json, "variant", &variant) != 0) {
        return NULL;
    }
    if (strcmp(variant, "binary4") == 0) {
        dev->variant = RATE_BINARY4;
        dev->bits = 4;
        dev->max = 15;
    } else if (strcmp(variant, "bcd") == 0) {
        dev->variant = RATE_BCD;
        dev->bits = 4;
        dev->max = 9;
    } else if (strcmp(variant, "binary6") == 0) {
        dev->variant = RATE_BINARY6;
        dev->bits = 6;
        dev->max = 63;
    } else {
        kind_error(ctx, "\"variant\" must be \"binary4\", \"bcd\" or \"binary6\", not \"%s\"",
                   variant);
        return NULL;
    }
    if (kind_string(ctx, json, "clock", &dev->clock) != 0 ||
        seq_names(ctx, json, "rate", dev->bits, dev->bits, &dev->rate, &dev->rate_count) != 0 ||
        seq_control_name(ctx, json, "enable", 1, 0, &dev->enable) != 0 ||
        seq_control_name(ctx, json, "strobe", 1, 0, &dev->strobe) != 0 ||
        seq_control_name(ctx, json, "cascade", 1, 0, &dev->cascade) != 0 ||
        seq_control_name(ctx, json, "clear", 1, 0, &dev->clear) != 0 ||
        seq_control_name(ctx, json, "set", 0, 0, &dev->set) != 0 ||
        kind_string(ctx, json, "out", &dev->out) != 0 ||
        kind_string(ctx, json, "outBar", &dev->out_bar) != 0 ||
        seq_optional_name(ctx, json, "terminal", &dev->terminal) != 0 ||
        kind_string(ctx, json, "enableOut", &dev->enable_out) != 0) {
        return NULL;
    }
    *units = 1;
    return dev;
}

static void *rate_bind(KindContext *ctx, const void *raw, const PartPin *pins, size_t count) {
    const RateDevice *d = raw;
    RateBound *b = kind_alloc(ctx, sizeof(*b));

    if (b == NULL || seq_bind(ctx, pins, count, d->clock, &b->clock) != 0 ||
        cdv_bind_names(ctx, pins, count, d->rate, d->bits, b->rate) != 0 ||
        seq_bind_control(ctx, pins, count, &d->enable, &b->enable) != 0 ||
        seq_bind_control(ctx, pins, count, &d->strobe, &b->strobe) != 0 ||
        seq_bind_control(ctx, pins, count, &d->cascade, &b->cascade) != 0 ||
        seq_bind_control(ctx, pins, count, &d->clear, &b->clear) != 0 ||
        seq_bind_control(ctx, pins, count, &d->set, &b->set) != 0 ||
        seq_bind(ctx, pins, count, d->out, &b->out) != 0 ||
        seq_bind(ctx, pins, count, d->out_bar, &b->out_bar) != 0 ||
        seq_bind(ctx, pins, count, d->terminal, &b->terminal) != 0 ||
        seq_bind(ctx, pins, count, d->enable_out, &b->enable_out) != 0) {
        return NULL;
    }
    b->dev = d;
    return b;
}

static CounterHeader *rate_create(const void *dev, const void *bound) {
    const RateBound *b = bound;
    RateState *s = logic_calloc(1, sizeof(*s));

    (void)dev;
    if (s == NULL) {
        return NULL;
    }
    s->b = b;
    cdv_init_outputs(&s->out, &b->out, 1);
    cdv_init_outputs(&s->out_bar, &b->out_bar, 1);
    cdv_init_outputs(&s->terminal, &b->terminal, 1);
    cdv_init_outputs(&s->enable_out, &b->enable_out, 1);
    return &s->h;
}

/* 1 when rate pin i (LSB first) selects count c. */
static int rate_selects(const RateDevice *d, unsigned i, int c) {
    if (d->variant == RATE_BCD) {
        static const unsigned short SETS[4] = {
            1u << 4,                                            /* A */
            (1u << 2) | (1u << 7),                              /* B */
            (1u << 1) | (1u << 3) | (1u << 6) | (1u << 8),      /* C */
            0x1EFu & ~(1u << 4)                                 /* D: 0-3, 5-8 */
        };

        return (SETS[i] >> c) & 1u;
    } else {
        unsigned j = d->bits - 1 - i;   /* from the MSB */
        unsigned mask = (1u << (j + 1)) - 1;

        return ((unsigned)c & mask) == (1u << j) - 1;
    }
}

static void rate_outputs(Sim *sim, SimInstance *inst, RateState *s) {
    const RateBound *b = s->b;
    const RateDevice *d = b->dev;
    const CounterTiming *t = s->h.timing;
    int strobe = seq_sync(sim, inst, &b->strobe);
    int clock = seq_read(sim, inst, b->clock);
    int cascade = seq_sync(sim, inst, &b->cascade);
    int enable = seq_sync(sim, inst, &b->enable);
    int any = 0;
    int at_max = s->count < 0 ? -1 : s->count == d->max;
    int g;
    unsigned i;

    for (i = 0; i < d->bits; i++) {
        int bit = seq_read(sim, inst, b->rate[i]);
        int sel;

        if (s->count >= 0) {
            sel = rate_selects(d, i, s->count);
        } else {
            int c, have = 0;

            sel = -1;
            for (c = 0; c <= d->max; c++) {
                int o = rate_selects(d, i, c);

                sel = !have ? o : sel == o ? sel : -1;
                have = 1;
            }
        }
        any = cdv_or3(any, cdv_and3(bit, sel));
    }
    g = cdv_and3(cdv_and3(cdv_not3(strobe), cdv_not3(clock)), any);
    cdv_drive(sim, inst, &s->out_bar, cdv_not3(g), t);
    cdv_drive(sim, inst, &s->out, cdv_or3(g, cascade), t);
    cdv_drive(sim, inst, &s->terminal, at_max, t);
    cdv_drive(sim, inst, &s->enable_out, cdv_not3(cdv_and3(enable, at_max)), t);
}

static void rate_eval(Sim *sim, SimInstance *inst, RateState *s, int at_start) {
    const RateBound *b = s->b;
    const RateDevice *d = b->dev;
    LogicValue clock = sim_read_pin(sim, inst, b->clock);
    int edge = !at_start && seq_rising(s->last_clock, clock);
    int set = b->set.pin != SEQ_NO_PIN ? seq_async(sim, inst, &b->set) : 0;
    int clear = seq_async(sim, inst, &b->clear);

    s->last_clock = clock;
    if (edge && s->count >= 0) {
        int en = seq_sync(sim, inst, &b->enable);
        int next = s->count == d->max ? 0 : s->count + 1;

        s->count = en == 1 ? next : en == 0 ? s->count : -1;
    } else if (edge) {
        s->count = -1;
    }
    if (set == 1) {
        s->count = d->max;
    } else if (clear == 1 && set == 0) {
        s->count = 0;
    } else if ((set < 0 && s->count != d->max) || (clear < 0 && s->count != 0) ||
               (clear == 1 && set < 0)) {
        s->count = -1;
    }
    rate_outputs(sim, inst, s);
}

static void rate_start(Sim *sim, SimInstance *inst, CounterHeader *h) {
    RateState *s = (RateState *)h;

    s->count = 0;
    s->last_clock = sim_read_pin(sim, inst, s->b->clock);
    rate_eval(sim, inst, s, 1);
}

static void rate_changed(Sim *sim, SimInstance *inst, CounterHeader *h, size_t pin,
                         LogicValue old_value, LogicValue new_value) {
    (void)pin;
    (void)old_value;
    (void)new_value;
    rate_eval(sim, inst, (RateState *)h, 0);
}

const CounterModel RATE_MODEL = {
    "rate", RATE_KEYS, NULL, rate_load, rate_bind, rate_create, NULL, rate_start, rate_changed,
    NULL
};
