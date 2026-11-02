/*
 * counter_div: the counter kind's dividing models (plan-10-E section 4).
 *
 *   johnson  Johnson (twisted-ring) counters with decoded outputs and the
 *            anti-lock gate D3 = Q2.(Q1 + Q3): 4017B, 4022B, 74HC4017, 4018B
 *   seg7     decade counters with 7-segment outputs: 4026B, 4033B, 40110B
 *   divn     programmable divide-by-N (master preset, extra count): 4059A
 *   rate     rate multipliers: 4089B, 4527B, 74x97
 *
 * counter.c holds the kind and lists these models (counter.h).
 */

#include <stdlib.h>
#include <string.h>

#include "counter.h"
#include "kind.h"
#include "seq.h"

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

static void drive(Sim *sim, SimInstance *inst, SeqOutput *o, int level, const CounterTiming *t) {
    counter_drive(sim, inst, o, seq_value(level), t, 0);
}

/* An optional {"in", "out"} pair of pin names. */
static int pin_pair(KindContext *ctx, const cJSON *json, const char *key, const char **in,
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

/* ---- johnson ---------------------------------------------------------------- */

#define JOHNSON_MAX 5

static const char *const JOHNSON_KEYS[] = {
    "stages", "clock", "clockGate", "reset", "outputs", "carry", "data", "presetEnable", "jam",
    "qbar", NULL
};

typedef struct {
    unsigned stages;
    const char *clock;
    SeqGateName gate;
    SeqControlName reset, preset;
    const char *const *outputs;
    size_t output_count;
    const char *carry, *data;
    const char *const *jam;
    size_t jam_count;
    const char *const *qbar;
    size_t qbar_count;
} JohnsonDevice;

typedef struct {
    const JohnsonDevice *dev;
    size_t clock;
    SeqGate gate;
    SeqControl reset, preset;
    size_t outputs[2 * JOHNSON_MAX];
    size_t carry, data;
    size_t jam[JOHNSON_MAX];
    size_t qbar[JOHNSON_MAX];
} JohnsonBound;

typedef struct {
    CounterHeader h;
    const JohnsonBound *b;
    int q[JOHNSON_MAX];          /* stage levels 1 / 0 / -1 */
    LogicValue last_clock;
    SeqOutput outputs[2 * JOHNSON_MAX];
    SeqOutput carry;
    SeqOutput qbar[JOHNSON_MAX];
} JohnsonState;

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
        bind_names(ctx, pins, count, dev->outputs, dev->output_count, b->outputs) != 0 ||
        seq_bind(ctx, pins, count, dev->carry, &b->carry) != 0 ||
        seq_bind(ctx, pins, count, dev->data, &b->data) != 0 ||
        bind_names(ctx, pins, count, dev->jam, dev->jam_count, b->jam) != 0 ||
        bind_names(ctx, pins, count, dev->qbar, dev->qbar_count, b->qbar) != 0) {
        return NULL;
    }
    b->dev = dev;
    return b;
}

static CounterHeader *johnson_create(const void *dev, const void *bound) {
    const JohnsonBound *b = bound;
    JohnsonState *s = calloc(1, sizeof(*s));

    (void)dev;
    if (s == NULL) {
        return NULL;
    }
    s->b = b;
    init_outputs(s->outputs, b->outputs, b->dev->output_count);
    init_outputs(&s->carry, &b->carry, 1);
    init_outputs(s->qbar, b->qbar, b->dev->qbar_count);
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
        next[0] = b->data != SEQ_NO_PIN ? seq_read(sim, inst, b->data) : not3(s->q[n - 1]);
        next[1] = s->q[0];
        next[2] = and3(s->q[1], or3(s->q[0], s->q[2]));
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

        drive(sim, inst, &s->outputs[0], and3(not3(q[0]), not3(q[n - 1])), s->h.timing);
        for (k = 1; k < n; k++) {
            drive(sim, inst, &s->outputs[k], and3(q[k - 1], not3(q[k])), s->h.timing);
        }
        drive(sim, inst, &s->outputs[n], and3(q[0], q[n - 1]), s->h.timing);
        for (k = 1; k < n; k++) {
            drive(sim, inst, &s->outputs[n + k], and3(not3(q[k - 1]), q[k]), s->h.timing);
        }
    }
    drive(sim, inst, &s->carry, not3(s->q[n - 1]), s->h.timing);
    for (k = 0; k < dev->qbar_count; k++) {
        drive(sim, inst, &s->qbar[k], not3(s->q[k]), s->h.timing);
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

/* ---- seg7 -------------------------------------------------------------------- */

static const char *const SEG7_KEYS[] = {
    "clock", "clockGate", "clockUp", "clockDown", "reset", "segments", "glyphs", "carry",
    "carryPulse", "borrowPulse", "displayEnable", "ungatedC", "lampTest", "rippleBlank",
    "toggleEnable", "latchEnable", NULL
};
static const CounterExtraKey SEG7_EXTRA[] = {{"carryPulse", 0}, {"borrowPulse", 0}, {NULL, 0}};

enum { SEG7_TIMER_CARRY = 1, SEG7_TIMER_BORROW = 2 };

typedef struct {
    const char *clock, *clock_up, *clock_down;
    SeqGateName gate;
    SeqControlName reset, lamp_test, toggle, latch;
    const char *const *segments;
    size_t segment_count;
    unsigned char glyphs[10];
    const char *carry, *carry_pulse, *borrow_pulse;
    const char *de_in, *de_out, *ungated_c, *rb_in, *rb_out;
} Seg7Device;

typedef struct {
    const Seg7Device *dev;
    size_t clock, clock_up, clock_down;
    SeqGate gate;
    SeqControl reset, lamp_test, toggle, latch;
    size_t segments[7];
    size_t carry, carry_pulse, borrow_pulse, de_in, de_out, ungated_c, rb_in, rb_out;
} Seg7Bound;

typedef struct {
    CounterHeader h;
    const Seg7Bound *b;
    int count;         /* 0-9, -1 unknown */
    int shown;         /* the count the segments show (latchEnable) */
    LogicValue last_clock, last_up, last_down;
    uint32_t carry_gen, borrow_gen;
    SeqOutput segments[7], carry, carry_pulse, borrow_pulse, de_out, ungated_c, rb_out;
} Seg7State;

static void *seg7_load(KindContext *ctx, const cJSON *json, unsigned *units) {
    Seg7Device *dev = kind_alloc(ctx, sizeof(*dev));
    const char *const *glyphs;
    size_t glyph_count;
    size_t i, j;

    if (dev == NULL) {
        return NULL;
    }
    if (cJSON_GetObjectItemCaseSensitive(json, "clockUp") != NULL) {
        if (kind_string(ctx, json, "clockUp", &dev->clock_up) != 0 ||
            kind_string(ctx, json, "clockDown", &dev->clock_down) != 0) {
            return NULL;
        }
    } else if (kind_string(ctx, json, "clock", &dev->clock) != 0) {
        return NULL;
    }
    if (seq_gate_name(ctx, json, "clockGate", &dev->gate) != 0 ||
        seq_control_name(ctx, json, "reset", 1, 0, &dev->reset) != 0 ||
        seq_control_name(ctx, json, "lampTest", 0, 0, &dev->lamp_test) != 0 ||
        seq_control_name(ctx, json, "toggleEnable", 0, 0, &dev->toggle) != 0 ||
        seq_control_name(ctx, json, "latchEnable", 0, 0, &dev->latch) != 0 ||
        seq_names(ctx, json, "segments", 7, 7, &dev->segments, &dev->segment_count) != 0 ||
        seq_names(ctx, json, "glyphs", 10, 10, &glyphs, &glyph_count) != 0 ||
        seq_optional_name(ctx, json, "carry", &dev->carry) != 0 ||
        seq_optional_name(ctx, json, "carryPulse", &dev->carry_pulse) != 0 ||
        seq_optional_name(ctx, json, "borrowPulse", &dev->borrow_pulse) != 0 ||
        seq_optional_name(ctx, json, "ungatedC", &dev->ungated_c) != 0 ||
        pin_pair(ctx, json, "displayEnable", &dev->de_in, &dev->de_out) != 0 ||
        pin_pair(ctx, json, "rippleBlank", &dev->rb_in, &dev->rb_out) != 0) {
        return NULL;
    }
    for (i = 0; i < 10; i++) {
        if (strlen(glyphs[i]) != 7) {
            kind_error(ctx, "glyphs[%zu]: must be 7 characters 0 or 1 (segments a to g)", i);
            return NULL;
        }
        for (j = 0; j < 7; j++) {
            if (glyphs[i][j] != '0' && glyphs[i][j] != '1') {
                kind_error(ctx, "glyphs[%zu]: must be 7 characters 0 or 1 (segments a to g)", i);
                return NULL;
            }
            if (glyphs[i][j] == '1') {
                dev->glyphs[i] |= (unsigned char)(1u << j);
            }
        }
    }
    *units = 1;
    return dev;
}

static void *seg7_bind(KindContext *ctx, const void *raw, const PartPin *pins, size_t count) {
    const Seg7Device *d = raw;
    Seg7Bound *b = kind_alloc(ctx, sizeof(*b));

    if (b == NULL || seq_bind(ctx, pins, count, d->clock, &b->clock) != 0 ||
        seq_bind(ctx, pins, count, d->clock_up, &b->clock_up) != 0 ||
        seq_bind(ctx, pins, count, d->clock_down, &b->clock_down) != 0 ||
        seq_bind_gate(ctx, pins, count, &d->gate, &b->gate) != 0 ||
        seq_bind_control(ctx, pins, count, &d->reset, &b->reset) != 0 ||
        seq_bind_control(ctx, pins, count, &d->lamp_test, &b->lamp_test) != 0 ||
        seq_bind_control(ctx, pins, count, &d->toggle, &b->toggle) != 0 ||
        seq_bind_control(ctx, pins, count, &d->latch, &b->latch) != 0 ||
        bind_names(ctx, pins, count, d->segments, 7, b->segments) != 0 ||
        seq_bind(ctx, pins, count, d->carry, &b->carry) != 0 ||
        seq_bind(ctx, pins, count, d->carry_pulse, &b->carry_pulse) != 0 ||
        seq_bind(ctx, pins, count, d->borrow_pulse, &b->borrow_pulse) != 0 ||
        seq_bind(ctx, pins, count, d->de_in, &b->de_in) != 0 ||
        seq_bind(ctx, pins, count, d->de_out, &b->de_out) != 0 ||
        seq_bind(ctx, pins, count, d->ungated_c, &b->ungated_c) != 0 ||
        seq_bind(ctx, pins, count, d->rb_in, &b->rb_in) != 0 ||
        seq_bind(ctx, pins, count, d->rb_out, &b->rb_out) != 0) {
        return NULL;
    }
    b->dev = d;
    return b;
}

static CounterHeader *seg7_create(const void *dev, const void *bound) {
    const Seg7Bound *b = bound;
    Seg7State *s = calloc(1, sizeof(*s));

    (void)dev;
    if (s == NULL) {
        return NULL;
    }
    s->b = b;
    init_outputs(s->segments, b->segments, 7);
    init_outputs(&s->carry, &b->carry, 1);
    init_outputs(&s->carry_pulse, &b->carry_pulse, 1);
    init_outputs(&s->borrow_pulse, &b->borrow_pulse, 1);
    init_outputs(&s->de_out, &b->de_out, 1);
    init_outputs(&s->ungated_c, &b->ungated_c, 1);
    init_outputs(&s->rb_out, &b->rb_out, 1);
    return &s->h;
}

static LogicValue seg7_clock(const Sim *sim, const SimInstance *inst, const Seg7Bound *b) {
    LogicValue clock = sim_read_pin(sim, inst, b->clock);

    if (b->gate.pin != SEQ_NO_PIN) {
        clock = seq_gated_clock(clock, sim_read_pin(sim, inst, b->gate.pin), b->gate.op);
    }
    return clock;
}

/* Segment j of a count (count -1 -> -1 unless every digit agrees). */
static int glyph_level(const Seg7Device *d, int count, unsigned j) {
    int have = 0, v = -1;
    int c;

    if (count >= 0) {
        return (d->glyphs[count] >> j) & 1u;
    }
    for (c = 0; c < 10; c++) {
        int o = (d->glyphs[c] >> j) & 1u;

        v = !have ? o : v == o ? v : -1;
        have = 1;
    }
    return v;
}

/* A one-shot low pulse on o: low now (fall delay), high again width_ps later. */
static void seg7_pulse(Sim *sim, SimInstance *inst, Seg7State *s, SeqOutput *o, uint32_t kind,
                       uint32_t *gen, SimTime width_ps) {
    if (o->pin == SEQ_NO_PIN) {
        return;
    }
    counter_drive(sim, inst, o, LOGIC_0, s->h.timing, 0);
    (*gen)++;
    sim_schedule_timer(sim, inst, (kind << 24) | (*gen & 0xFFFFFFu),
                       s->h.timing->seq->fall + width_ps);
}

static void seg7_outputs(Sim *sim, SimInstance *inst, Seg7State *s) {
    const Seg7Bound *b = s->b;
    const Seg7Device *d = b->dev;
    const CounterTiming *t = s->h.timing;
    int lamp = seq_sync(sim, inst, &b->lamp_test);
    int de = b->de_in != SEQ_NO_PIN ? seq_read(sim, inst, b->de_in) : 1;
    int is_zero = s->shown < 0 ? -1 : s->shown == 0;
    int rbi = b->rb_in != SEQ_NO_PIN ? seq_read(sim, inst, b->rb_in) : 1;
    int blank = or3(not3(de), and3(is_zero, not3(rbi)));
    unsigned j;

    if (b->lamp_test.pin == SEQ_NO_PIN) {
        lamp = 0;
    }
    for (j = 0; j < 7; j++) {
        int seg = or3(lamp, and3(not3(blank), glyph_level(d, s->shown, j)));

        drive(sim, inst, &s->segments[j], seg, t);
    }
    drive(sim, inst, &s->carry, s->count < 0 ? -1 : s->count <= 4, t);
    drive(sim, inst, &s->de_out, de, t);
    drive(sim, inst, &s->ungated_c, glyph_level(d, s->count, 2), t);
    drive(sim, inst, &s->rb_out, not3(and3(s->count < 0 ? -1 : s->count == 0, not3(rbi))), t);
}

static void seg7_eval(Sim *sim, SimInstance *inst, Seg7State *s, int at_start) {
    const Seg7Bound *b = s->b;
    const Seg7Device *d = b->dev;
    int reset = seq_async(sim, inst, &b->reset);
    int toggle = b->toggle.pin != SEQ_NO_PIN ? seq_sync(sim, inst, &b->toggle) : 1;
    int step = 0;   /* +1 up, -1 down, 0 none; 2 = unknown */
    int before = s->count;
    int latch;

    if (d->clock_up != NULL) {
        LogicValue up = sim_read_pin(sim, inst, b->clock_up);
        LogicValue down = sim_read_pin(sim, inst, b->clock_down);

        if (!at_start && seq_rising(s->last_up, up)) {
            step += 1;
        }
        if (!at_start && seq_rising(s->last_down, down)) {
            step -= 1;
        }
        s->last_up = up;
        s->last_down = down;
    } else {
        LogicValue clock = seg7_clock(sim, inst, b);

        if (!at_start && seq_rising(s->last_clock, clock)) {
            step = 1;
        }
        s->last_clock = clock;
    }
    if (step != 0 && s->count >= 0) {
        int stepped = (s->count + 10 + step) % 10;

        if (toggle == 1) {
            s->count = stepped;
        } else if (toggle < 0) {
            s->count = -1;
        }
    }
    if (reset == 1) {
        s->count = 0;
    } else if (reset < 0 && s->count != 0) {
        s->count = -1;
    }
    latch = b->latch.pin != SEQ_NO_PIN ? seq_sync(sim, inst, &b->latch) : 0;
    if (latch == 0) {
        s->shown = s->count;
    } else if (latch < 0 && s->shown != s->count) {
        s->shown = -1;
    }
    if (!at_start && reset != 1 && before >= 0 && s->count >= 0) {
        if (before == 9 && s->count == 0 && step > 0) {
            seg7_pulse(sim, inst, s, &s->carry_pulse, SEG7_TIMER_CARRY, &s->carry_gen,
                       s->h.timing->extra[0]);
        } else if (before == 0 && s->count == 9 && step < 0) {
            seg7_pulse(sim, inst, s, &s->borrow_pulse, SEG7_TIMER_BORROW, &s->borrow_gen,
                       s->h.timing->extra[1]);
        }
    }
    if (at_start) {
        counter_drive(sim, inst, &s->carry_pulse, LOGIC_1, s->h.timing, 0);
        counter_drive(sim, inst, &s->borrow_pulse, LOGIC_1, s->h.timing, 0);
    }
    seg7_outputs(sim, inst, s);
}

static void seg7_start(Sim *sim, SimInstance *inst, CounterHeader *h) {
    Seg7State *s = (Seg7State *)h;
    const Seg7Bound *b = s->b;

    s->count = 0;
    s->shown = 0;
    if (b->dev->clock_up != NULL) {
        s->last_up = sim_read_pin(sim, inst, b->clock_up);
        s->last_down = sim_read_pin(sim, inst, b->clock_down);
    } else {
        s->last_clock = seg7_clock(sim, inst, b);
    }
    seg7_eval(sim, inst, s, 1);
}

static void seg7_changed(Sim *sim, SimInstance *inst, CounterHeader *h, size_t pin,
                         LogicValue old_value, LogicValue new_value) {
    (void)pin;
    (void)old_value;
    (void)new_value;
    seg7_eval(sim, inst, (Seg7State *)h, 0);
}

static void seg7_timer(Sim *sim, SimInstance *inst, CounterHeader *h, uint32_t id) {
    Seg7State *s = (Seg7State *)h;
    SeqTiming now = *s->h.timing->seq;
    CounterTiming immediate = {&now, {0}};
    uint32_t kind = id >> 24;
    uint32_t gen = id & 0xFFFFFFu;

    now.rise = 0;
    now.fall = 0;
    if (kind == SEG7_TIMER_CARRY && gen == (s->carry_gen & 0xFFFFFFu)) {
        counter_drive(sim, inst, &s->carry_pulse, LOGIC_1, &immediate, 0);
    } else if (kind == SEG7_TIMER_BORROW && gen == (s->borrow_gen & 0xFFFFFFu)) {
        counter_drive(sim, inst, &s->borrow_pulse, LOGIC_1, &immediate, 0);
    }
}

const CounterModel SEG7_MODEL = {
    "seg7", SEG7_KEYS, SEG7_EXTRA, seg7_load, seg7_bind, seg7_create, NULL, seg7_start,
    seg7_changed, seg7_timer
};

/* ---- divn ---------------------------------------------------------------------- */

static const char *const DIVN_KEYS[] = {"clock", "jam", "modes", "latchEnable", "output", NULL};

typedef struct {
    const char *clock, *latch, *output;
    const char *const *jam;
    size_t jam_count;
    const char *const *modes;
    size_t mode_count;
} DivnDevice;

typedef struct {
    size_t clock, latch, output;
    size_t jam[16];
    size_t modes[3];
} DivnBound;

typedef struct {
    CounterHeader h;
    const DivnBound *b;
    long count;        /* C; -1 unknown */
    int preset;        /* the extra count after a master preset is pending */
    int pulse;         /* 1 / 0 / -1 for the current clock period */
    int latched;       /* latch enable has caught a pulse */
    LogicValue last_clock;
    SeqOutput out;
} DivnState;

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
        bind_names(ctx, pins, count, d->jam, 16, b->jam) != 0 ||
        bind_names(ctx, pins, count, d->modes, 3, b->modes) != 0) {
        return NULL;
    }
    return b;
}

static CounterHeader *divn_create(const void *dev, const void *bound) {
    const DivnBound *b = bound;
    DivnState *s = calloc(1, sizeof(*s));

    (void)dev;
    if (s == NULL) {
        return NULL;
    }
    s->b = b;
    init_outputs(&s->out, &b->output, 1);
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
    level = or3(s->pulse, and3(s->latched ? 1 : 0, le));
    drive(sim, inst, &s->out, level, s->h.timing);
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

enum { RATE_BINARY4, RATE_BCD, RATE_BINARY6 };

typedef struct {
    int variant;
    unsigned bits;       /* rate pins */
    int max;             /* 15, 9, 63 */
    const char *clock;
    const char *const *rate;
    size_t rate_count;
    SeqControlName enable, strobe, cascade, clear, set;
    const char *out, *out_bar, *terminal, *enable_out;
} RateDevice;

typedef struct {
    const RateDevice *dev;
    size_t clock;
    size_t rate[6];
    SeqControl enable, strobe, cascade, clear, set;
    size_t out, out_bar, terminal, enable_out;
} RateBound;

typedef struct {
    CounterHeader h;
    const RateBound *b;
    int count;   /* -1 unknown */
    LogicValue last_clock;
    SeqOutput out, out_bar, terminal, enable_out;
} RateState;

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
        bind_names(ctx, pins, count, d->rate, d->bits, b->rate) != 0 ||
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
    RateState *s = calloc(1, sizeof(*s));

    (void)dev;
    if (s == NULL) {
        return NULL;
    }
    s->b = b;
    init_outputs(&s->out, &b->out, 1);
    init_outputs(&s->out_bar, &b->out_bar, 1);
    init_outputs(&s->terminal, &b->terminal, 1);
    init_outputs(&s->enable_out, &b->enable_out, 1);
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
        any = or3(any, and3(bit, sel));
    }
    g = and3(and3(not3(strobe), not3(clock)), any);
    drive(sim, inst, &s->out_bar, not3(g), t);
    drive(sim, inst, &s->out, or3(g, cascade), t);
    drive(sim, inst, &s->terminal, at_max, t);
    drive(sim, inst, &s->enable_out, not3(and3(enable, at_max)), t);
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
