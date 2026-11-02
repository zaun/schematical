/*
 * latch: level-sensitive storage (plan-10-B section 5). One kind, five
 * models, picked by the device's "model" key:
 *
 *   transparent   D latches (74x373 and kin, 74x75, 4042B, 4508B)
 *   addressable   8-bit addressable latches (74x259, 4099B, TPIC6259)
 *   sr            S-R latches (74x279, 4043B, 4044B)
 *   bcd7seg       BCD latch + 7-segment decoder (74HC4511)
 *   bus-mux       12-to-24-bit multiplexed bus latch (74ALVCH16260)
 *
 * Every model is level-sensitive, so each re-evaluates all of its state on
 * any pin change. At start only the asynchronous controls act (power-on
 * state 0, then clear/preset/set/reset), so floating enables do not turn
 * the power-on 0s into X. Unknown inputs are handled by trying both levels of each
 * unknown control: a bit whose outcomes differ (or is X in one) becomes X.
 * Signal rules: docs/parts-format.md "Sequential kinds".
 */

#include <stdlib.h>
#include <string.h>

#include "kind.h"
#include "model_pins.h"
#include "seq.h"

#define LATCH_MAX_UNITS 40
#define MUX_CHANNELS 12

/* ---- Model table ---------------------------------------------------------- */

typedef struct LatchModel LatchModel;

/* Every model's run state starts with this. */
typedef struct {
    const LatchModel *model;
    const SeqTiming *timing;
} LatchHeader;

struct LatchModel {
    const char *name;
    const char *const *keys;
    void *(*load)(KindContext *ctx, const cJSON *json, unsigned *units);
    void *(*bind)(KindContext *ctx, const void *dev, const PartPin *pins, size_t count);
    LatchHeader *(*create)(const void *dev, const void *bound);
    /* at_start: only the asynchronous controls change state. */
    void (*eval)(Sim *sim, SimInstance *inst, LatchHeader *state, int at_start);
    int (*open_drain)(const void *dev, const void *bound, size_t pin);
};

typedef struct {
    const LatchModel *model;
    void *dev;
    SeqTimingTable timing;
} LatchDevice;

/* Outcome of trying each level of the unknown inputs. */
typedef struct {
    int have;
    LogicValue value;
} Merge;

static void merge(Merge *m, LogicValue v) {
    if (!m->have) {
        m->have = 1;
        m->value = v;
    } else if (m->value != v) {
        m->value = LOGIC_X;
    }
}

static LogicValue merged(const Merge *m) {
    return m->have ? m->value : LOGIC_X;
}

/* The levels an input may have: both when unknown. */
static int lo(int level) {
    return level < 0 ? 0 : level;
}

static int hi(int level) {
    return level < 0 ? 1 : level;
}

static LogicValue d_value(const Sim *sim, const SimInstance *inst, size_t pin) {
    return seq_value(seq_read(sim, inst, pin));
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

/* ---- transparent ---------------------------------------------------------- */

static const char *const TRANSPARENT_KEYS[] = {
    "units", "enableActive", "polarity", "outputEnableActive", "presetActive", "clearActive",
    "presetWins", "output", NULL
};

typedef struct {
    const char *d, *le, *q, *qn, *oe, *pre, *clr;
} TlUnitNames;

typedef struct {
    TlUnitNames units[LATCH_MAX_UNITS];
    size_t unit_count;
    int enable_high;
    const char *polarity;
    int oe_high, pre_high, clr_high;
    int preset_wins;
    SeqOutputStyle style;
} TlDevice;

typedef struct {
    size_t d, q, qn;
    SeqControl le, oe, pre, clr;
} TlUnit;

typedef struct {
    TlUnit units[LATCH_MAX_UNITS];
    size_t unit_count;
    size_t polarity;
    int preset_wins;
    SeqOutputStyle style;
} TlBound;

typedef struct {
    LatchHeader h;
    const TlBound *b;
    LogicValue stored[LATCH_MAX_UNITS];
    SeqOutput q[LATCH_MAX_UNITS];
    SeqOutput qn[LATCH_MAX_UNITS];
} TlState;

static void *tl_load(KindContext *ctx, const cJSON *json, unsigned *units_out) {
    static const char *const UNIT_KEYS[] = {"d", "le", "q", "qn", "oe", "pre", "clr", NULL};
    TlDevice *dev = kind_alloc(ctx, sizeof(*dev));
    const cJSON *units = kind_member(ctx, json, "units", 1);
    const cJSON *item;
    int any_oe = 0, any_pre = 0, any_clr = 0, any_both = 0;
    size_t n = 0;

    if (dev == NULL || units == NULL) {
        return NULL;
    }
    if (!cJSON_IsArray(units) || cJSON_GetArraySize(units) < 1 ||
        cJSON_GetArraySize(units) > LATCH_MAX_UNITS) {
        kind_error(ctx, "\"units\" must be an array of 1 to %d units", LATCH_MAX_UNITS);
        return NULL;
    }
    cJSON_ArrayForEach(item, units) {
        TlUnitNames *u = &dev->units[n];

        kind_push(ctx, "units[%zu]", n);
        if (kind_check_keys(ctx, item, UNIT_KEYS, NULL) != 0 ||
            kind_string(ctx, item, "d", &u->d) != 0 || kind_string(ctx, item, "le", &u->le) != 0 ||
            seq_optional_name(ctx, item, "q", &u->q) != 0 ||
            seq_optional_name(ctx, item, "qn", &u->qn) != 0 ||
            seq_optional_name(ctx, item, "oe", &u->oe) != 0 ||
            seq_optional_name(ctx, item, "pre", &u->pre) != 0 ||
            seq_optional_name(ctx, item, "clr", &u->clr) != 0) {
            return NULL;
        }
        if (u->q == NULL && u->qn == NULL) {
            kind_error(ctx, "a unit needs \"q\" or \"qn\"");
            return NULL;
        }
        any_oe |= u->oe != NULL;
        any_pre |= u->pre != NULL;
        any_clr |= u->clr != NULL;
        any_both |= u->pre != NULL && u->clr != NULL;
        kind_pop(ctx);
        n++;
    }
    dev->unit_count = n;
    if (cJSON_GetObjectItemCaseSensitive(json, "polarity") != NULL) {
        if (cJSON_GetObjectItemCaseSensitive(json, "enableActive") != NULL) {
            kind_error(ctx, "\"enableActive\" and \"polarity\" cannot both be given");
            return NULL;
        }
        if (kind_string(ctx, json, "polarity", &dev->polarity) != 0) {
            return NULL;
        }
    } else if (seq_active(ctx, json, "enableActive", &dev->enable_high) != 0) {
        return NULL;
    }
    if ((any_oe && seq_active(ctx, json, "outputEnableActive", &dev->oe_high) != 0) ||
        (any_pre && seq_active(ctx, json, "presetActive", &dev->pre_high) != 0) ||
        (any_clr && seq_active(ctx, json, "clearActive", &dev->clr_high) != 0) ||
        (any_both && kind_bool(ctx, json, "presetWins", &dev->preset_wins) != 0) ||
        seq_output_style(ctx, json, &dev->style) != 0) {
        return NULL;
    }
    *units_out = (unsigned)n;
    return dev;
}

static int tl_control(KindContext *ctx, const PartPin *pins, size_t count, const char *name,
                      int active_high, SeqControl *c) {
    SeqControlName cn = {name, active_high, 0};

    return seq_bind_control(ctx, pins, count, &cn, c);
}

static void *tl_bind(KindContext *ctx, const void *raw, const PartPin *pins, size_t count) {
    const TlDevice *dev = raw;
    TlBound *b = kind_alloc(ctx, sizeof(*b));
    size_t i;

    if (b == NULL) {
        return NULL;
    }
    for (i = 0; i < dev->unit_count; i++) {
        const TlUnitNames *n = &dev->units[i];
        TlUnit *u = &b->units[i];

        kind_push(ctx, "units[%zu]", i);
        if (seq_bind(ctx, pins, count, n->d, &u->d) != 0 ||
            seq_bind(ctx, pins, count, n->q, &u->q) != 0 ||
            seq_bind(ctx, pins, count, n->qn, &u->qn) != 0 ||
            tl_control(ctx, pins, count, n->le, dev->enable_high, &u->le) != 0 ||
            tl_control(ctx, pins, count, n->oe, dev->oe_high, &u->oe) != 0 ||
            tl_control(ctx, pins, count, n->pre, dev->pre_high, &u->pre) != 0 ||
            tl_control(ctx, pins, count, n->clr, dev->clr_high, &u->clr) != 0) {
            return NULL;
        }
        kind_pop(ctx);
    }
    if (seq_bind(ctx, pins, count, dev->polarity, &b->polarity) != 0) {
        return NULL;
    }
    b->unit_count = dev->unit_count;
    b->preset_wins = dev->preset_wins;
    b->style = dev->style;
    return b;
}

static LatchHeader *tl_create(const void *dev, const void *bound) {
    const TlBound *b = bound;
    TlState *s = calloc(1, sizeof(*s));
    size_t i;

    (void)dev;
    if (s == NULL) {
        return NULL;
    }
    s->b = b;
    for (i = 0; i < b->unit_count; i++) {
        s->stored[i] = LOGIC_0;
        init_outputs(&s->q[i], &b->units[i].q, 1);
        init_outputs(&s->qn[i], &b->units[i].qn, 1);
    }
    return &s->h;
}

/* 1 transparent, 0 latched, -1 unknown. */
static int tl_open(const Sim *sim, const SimInstance *inst, const TlBound *b, const TlUnit *u) {
    if (b->polarity != SEQ_NO_PIN) {
        int clock = seq_read(sim, inst, u->le.pin);
        int polarity = seq_read(sim, inst, b->polarity);

        return clock < 0 || polarity < 0 ? -1 : clock == polarity;
    }
    return seq_sync(sim, inst, &u->le);
}

static void tl_eval(Sim *sim, SimInstance *inst, LatchHeader *h, int at_start) {
    TlState *s = (TlState *)h;
    const TlBound *b = s->b;
    size_t i;

    for (i = 0; i < b->unit_count; i++) {
        const TlUnit *u = &b->units[i];
        int pre = seq_async(sim, inst, &u->pre);
        int clr = seq_async(sim, inst, &u->clr);
        int open = at_start ? 0 : tl_open(sim, inst, b, u);
        LogicValue d = d_value(sim, inst, u->d);
        Merge m = {0, LOGIC_X};
        int p, c, e;
        int enable;

        for (p = lo(pre); p <= hi(pre); p++) {
            for (c = lo(clr); c <= hi(clr); c++) {
                for (e = lo(open); e <= hi(open); e++) {
                    if (p && c) {
                        merge(&m, b->preset_wins ? LOGIC_1 : LOGIC_0);
                    } else if (c) {
                        merge(&m, LOGIC_0);
                    } else if (p) {
                        merge(&m, LOGIC_1);
                    } else {
                        merge(&m, e ? d : s->stored[i]);
                    }
                }
            }
        }
        s->stored[i] = merged(&m);
        enable = seq_enable(sim, inst, &u->oe);
        seq_drive(sim, inst, &s->q[i], seq_gate(s->stored[i], enable), b->style, h->timing);
        seq_drive(sim, inst, &s->qn[i], seq_gate(seq_not(s->stored[i]), enable), b->style,
                  h->timing);
    }
}

static int tl_open_drain(const void *dev, const void *bound, size_t pin) {
    const TlBound *b = bound;
    size_t i;

    (void)dev;
    if (b->style != SEQ_OPEN_DRAIN) {
        return 0;
    }
    for (i = 0; i < b->unit_count; i++) {
        if (pin == b->units[i].q || pin == b->units[i].qn) {
            return 1;
        }
    }
    return 0;
}

/* ---- addressable ---------------------------------------------------------- */

static const char *const ADDRESSABLE_KEYS[] = {
    "address", "data", "outputs", "enable", "clear", "output", "invert", NULL
};

typedef struct {
    const char *const *address;
    size_t address_count;
    const char *data;
    const char *const *outputs;
    size_t output_count;
    SeqControlName enable, clear;
    SeqOutputStyle style;
    int invert;
} AdDevice;

typedef struct {
    size_t address[3];
    size_t address_count;
    size_t data;
    size_t outputs[8];
    size_t output_count;
    SeqControl enable, clear;
    SeqOutputStyle style;
    int invert;
} AdBound;

typedef struct {
    LatchHeader h;
    const AdBound *b;
    LogicValue bit[8];
    SeqOutput out[8];
} AdState;

static void *ad_load(KindContext *ctx, const cJSON *json, unsigned *units) {
    AdDevice *dev = kind_alloc(ctx, sizeof(*dev));

    if (dev == NULL ||
        seq_names(ctx, json, "address", 1, 3, &dev->address, &dev->address_count) != 0 ||
        kind_string(ctx, json, "data", &dev->data) != 0 ||
        kind_names(ctx, json, "outputs", &dev->outputs, &dev->output_count) != 0 ||
        seq_control_name(ctx, json, "enable", 1, 0, &dev->enable) != 0 ||
        seq_control_name(ctx, json, "clear", 0, 0, &dev->clear) != 0 ||
        seq_output_style(ctx, json, &dev->style) != 0 ||
        seq_optional_bool(ctx, json, "invert", 0, &dev->invert) != 0) {
        return NULL;
    }
    if (dev->output_count != ((size_t)1 << dev->address_count)) {
        kind_error(ctx, "\"outputs\" must have %zu names (2 to the number of address pins)",
                   (size_t)1 << dev->address_count);
        return NULL;
    }
    *units = (unsigned)dev->output_count;
    return dev;
}

static void *ad_bind(KindContext *ctx, const void *raw, const PartPin *pins, size_t count) {
    const AdDevice *dev = raw;
    AdBound *b = kind_alloc(ctx, sizeof(*b));

    if (b == NULL ||
        bind_names(ctx, pins, count, dev->address, dev->address_count, b->address) != 0 ||
        bind_names(ctx, pins, count, dev->outputs, dev->output_count, b->outputs) != 0 ||
        seq_bind(ctx, pins, count, dev->data, &b->data) != 0 ||
        seq_bind_control(ctx, pins, count, &dev->enable, &b->enable) != 0 ||
        seq_bind_control(ctx, pins, count, &dev->clear, &b->clear) != 0) {
        return NULL;
    }
    b->address_count = dev->address_count;
    b->output_count = dev->output_count;
    b->style = dev->style;
    b->invert = dev->invert;
    return b;
}

static LatchHeader *ad_create(const void *dev, const void *bound) {
    const AdBound *b = bound;
    AdState *s = calloc(1, sizeof(*s));
    size_t i;

    (void)dev;
    if (s == NULL) {
        return NULL;
    }
    s->b = b;
    for (i = 0; i < b->output_count; i++) {
        s->bit[i] = LOGIC_0;
    }
    init_outputs(s->out, b->outputs, b->output_count);
    return &s->h;
}

static void ad_eval(Sim *sim, SimInstance *inst, LatchHeader *h, int at_start) {
    AdState *s = (AdState *)h;
    const AdBound *b = s->b;
    int enable = at_start ? 0 : seq_sync(sim, inst, &b->enable);
    int clear = seq_async(sim, inst, &b->clear);
    LogicValue d = d_value(sim, inst, b->data);
    unsigned known = 0, value = 0;
    size_t i;

    for (i = 0; i < b->address_count; i++) {
        int level = seq_read(sim, inst, b->address[i]);

        if (level >= 0) {
            known |= 1u << i;
            value |= (unsigned)level << i;
        }
    }
    for (i = 0; i < b->output_count; i++) {
        Merge m = {0, LOGIC_X};
        unsigned addr;
        int e, c;

        for (addr = 0; addr < b->output_count; addr++) {
            if ((addr & known) != value) {
                continue;
            }
            for (e = lo(enable); e <= hi(enable); e++) {
                for (c = lo(clear); c <= hi(clear); c++) {
                    if (e && addr == i) {
                        merge(&m, d);
                    } else if (c) {
                        merge(&m, LOGIC_0);
                    } else {
                        merge(&m, s->bit[i]);
                    }
                }
            }
        }
        s->bit[i] = merged(&m);
        seq_drive(sim, inst, &s->out[i], b->invert ? seq_not(s->bit[i]) : s->bit[i], b->style,
                  h->timing);
    }
}

static int ad_open_drain(const void *dev, const void *bound, size_t pin) {
    const AdBound *b = bound;
    size_t i;

    (void)dev;
    for (i = 0; b->style == SEQ_OPEN_DRAIN && i < b->output_count; i++) {
        if (pin == b->outputs[i]) {
            return 1;
        }
    }
    return 0;
}

/* ---- sr ------------------------------------------------------------------- */

static const char *const SR_KEYS[] = {"units", "inputsActive", "both", "outputEnable", NULL};

#define SR_MAX_UNITS 8

typedef struct {
    const char *const *s;
    size_t s_count;
    const char *r, *q;
} SrUnitNames;

typedef struct {
    SrUnitNames units[SR_MAX_UNITS];
    size_t unit_count;
    int active_high;
    int both_set;
    SeqControlName oe;
} SrDevice;

typedef struct {
    SeqControl s[2];
    size_t s_count;
    SeqControl r;
    size_t q;
} SrUnit;

typedef struct {
    SrUnit units[SR_MAX_UNITS];
    size_t unit_count;
    int both_set;
    SeqControl oe;
} SrBound;

typedef struct {
    LatchHeader h;
    const SrBound *b;
    LogicValue stored[SR_MAX_UNITS];
    SeqOutput q[SR_MAX_UNITS];
} SrState;

static void *sr_load(KindContext *ctx, const cJSON *json, unsigned *units_out) {
    static const char *const UNIT_KEYS[] = {"s", "r", "q", NULL};
    SrDevice *dev = kind_alloc(ctx, sizeof(*dev));
    const cJSON *units = kind_member(ctx, json, "units", 1);
    const cJSON *item;
    const char *both;
    size_t n = 0;

    if (dev == NULL || units == NULL) {
        return NULL;
    }
    if (!cJSON_IsArray(units) || cJSON_GetArraySize(units) < 1 ||
        cJSON_GetArraySize(units) > SR_MAX_UNITS) {
        kind_error(ctx, "\"units\" must be an array of 1 to %d units", SR_MAX_UNITS);
        return NULL;
    }
    cJSON_ArrayForEach(item, units) {
        SrUnitNames *u = &dev->units[n];

        kind_push(ctx, "units[%zu]", n);
        if (kind_check_keys(ctx, item, UNIT_KEYS, NULL) != 0 ||
            seq_names(ctx, item, "s", 1, 2, &u->s, &u->s_count) != 0 ||
            kind_string(ctx, item, "r", &u->r) != 0 || kind_string(ctx, item, "q", &u->q) != 0) {
            return NULL;
        }
        kind_pop(ctx);
        n++;
    }
    dev->unit_count = n;
    if (seq_active(ctx, json, "inputsActive", &dev->active_high) != 0 ||
        kind_string(ctx, json, "both", &both) != 0 ||
        seq_control_name(ctx, json, "outputEnable", 0, 0, &dev->oe) != 0) {
        return NULL;
    }
    if (strcmp(both, "set") == 0) {
        dev->both_set = 1;
    } else if (strcmp(both, "reset") != 0) {
        kind_error(ctx, "\"both\" must be \"set\" or \"reset\", not \"%s\"", both);
        return NULL;
    }
    *units_out = (unsigned)n;
    return dev;
}

static void *sr_bind(KindContext *ctx, const void *raw, const PartPin *pins, size_t count) {
    const SrDevice *dev = raw;
    SrBound *b = kind_alloc(ctx, sizeof(*b));
    size_t i, j;

    if (b == NULL) {
        return NULL;
    }
    for (i = 0; i < dev->unit_count; i++) {
        const SrUnitNames *n = &dev->units[i];
        SrUnit *u = &b->units[i];
        SeqControlName r = {n->r, dev->active_high, 0};

        kind_push(ctx, "units[%zu]", i);
        for (j = 0; j < n->s_count; j++) {
            SeqControlName s = {n->s[j], dev->active_high, 0};

            if (seq_bind_control(ctx, pins, count, &s, &u->s[j]) != 0) {
                return NULL;
            }
        }
        u->s_count = n->s_count;
        if (seq_bind_control(ctx, pins, count, &r, &u->r) != 0 ||
            seq_bind(ctx, pins, count, n->q, &u->q) != 0) {
            return NULL;
        }
        kind_pop(ctx);
    }
    if (seq_bind_control(ctx, pins, count, &dev->oe, &b->oe) != 0) {
        return NULL;
    }
    b->unit_count = dev->unit_count;
    b->both_set = dev->both_set;
    return b;
}

static LatchHeader *sr_create(const void *dev, const void *bound) {
    const SrBound *b = bound;
    SrState *s = calloc(1, sizeof(*s));
    size_t i;

    (void)dev;
    if (s == NULL) {
        return NULL;
    }
    s->b = b;
    for (i = 0; i < b->unit_count; i++) {
        s->stored[i] = LOGIC_0;
        init_outputs(&s->q[i], &b->units[i].q, 1);
    }
    return &s->h;
}

static void sr_eval(Sim *sim, SimInstance *inst, LatchHeader *h, int at_start) {
    SrState *s = (SrState *)h;
    const SrBound *b = s->b;
    int enable = seq_enable(sim, inst, &b->oe);
    size_t i, j;

    (void)at_start;   /* S and R are asynchronous: they act at start too. */

    for (i = 0; i < b->unit_count; i++) {
        const SrUnit *u = &b->units[i];
        int set = 0;
        int reset = seq_async(sim, inst, &u->r);
        Merge m = {0, LOGIC_X};
        int sv, rv;

        for (j = 0; j < u->s_count; j++) {
            int level = seq_async(sim, inst, &u->s[j]);

            if (level == 1) {
                set = 1;
            } else if (level < 0 && set == 0) {
                set = -1;
            }
        }
        for (sv = lo(set); sv <= hi(set); sv++) {
            for (rv = lo(reset); rv <= hi(reset); rv++) {
                if (sv && rv) {
                    merge(&m, b->both_set ? LOGIC_1 : LOGIC_0);
                } else if (sv) {
                    merge(&m, LOGIC_1);
                } else if (rv) {
                    merge(&m, LOGIC_0);
                } else {
                    merge(&m, s->stored[i]);
                }
            }
        }
        s->stored[i] = merged(&m);
        seq_drive(sim, inst, &s->q[i], seq_gate(s->stored[i], enable), SEQ_PUSH_PULL, h->timing);
    }
}

/* ---- bcd7seg -------------------------------------------------------------- */

static const char *const BCD_KEYS[] = {
    "inputs", "latch", "lampTest", "blank", "segments", "glyphs", NULL
};

typedef struct {
    const char *const *inputs;
    size_t input_count;
    const char *const *segments;
    size_t segment_count;
    SeqControlName latch, lamp_test, blank;
    unsigned char glyphs[16];   /* bit j = segment j (a = bit 0) */
} BcdDevice;

typedef struct {
    size_t inputs[4];
    size_t segments[7];
    SeqControl latch, lamp_test, blank;
    const unsigned char *glyphs;
} BcdBound;

typedef struct {
    LatchHeader h;
    const BcdBound *b;
    LogicValue code[4];
    SeqOutput seg[7];
} BcdState;

static void *bcd_load(KindContext *ctx, const cJSON *json, unsigned *units) {
    BcdDevice *dev = kind_alloc(ctx, sizeof(*dev));
    const char *const *glyphs;
    size_t glyph_count;
    size_t i, j;

    if (dev == NULL || seq_names(ctx, json, "inputs", 4, 4, &dev->inputs, &dev->input_count) != 0 ||
        seq_names(ctx, json, "segments", 7, 7, &dev->segments, &dev->segment_count) != 0 ||
        seq_control_name(ctx, json, "latch", 1, 0, &dev->latch) != 0 ||
        seq_control_name(ctx, json, "lampTest", 0, 0, &dev->lamp_test) != 0 ||
        seq_control_name(ctx, json, "blank", 0, 0, &dev->blank) != 0 ||
        seq_names(ctx, json, "glyphs", 16, 16, &glyphs, &glyph_count) != 0) {
        return NULL;
    }
    for (i = 0; i < 16; i++) {
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

static void *bcd_bind(KindContext *ctx, const void *raw, const PartPin *pins, size_t count) {
    const BcdDevice *dev = raw;
    BcdBound *b = kind_alloc(ctx, sizeof(*b));

    if (b == NULL || bind_names(ctx, pins, count, dev->inputs, 4, b->inputs) != 0 ||
        bind_names(ctx, pins, count, dev->segments, 7, b->segments) != 0 ||
        seq_bind_control(ctx, pins, count, &dev->latch, &b->latch) != 0 ||
        seq_bind_control(ctx, pins, count, &dev->lamp_test, &b->lamp_test) != 0 ||
        seq_bind_control(ctx, pins, count, &dev->blank, &b->blank) != 0) {
        return NULL;
    }
    b->glyphs = dev->glyphs;
    return b;
}

static LatchHeader *bcd_create(const void *dev, const void *bound) {
    const BcdBound *b = bound;
    BcdState *s = calloc(1, sizeof(*s));
    size_t i;

    (void)dev;
    if (s == NULL) {
        return NULL;
    }
    s->b = b;
    for (i = 0; i < 4; i++) {
        s->code[i] = LOGIC_0;
    }
    init_outputs(s->seg, b->segments, 7);
    return &s->h;
}

static void bcd_eval(Sim *sim, SimInstance *inst, LatchHeader *h, int at_start) {
    BcdState *s = (BcdState *)h;
    const BcdBound *b = s->b;
    int hold = at_start ? 1 : seq_sync(sim, inst, &b->latch);
    int lamp = b->lamp_test.pin == SEQ_NO_PIN ? 0 : seq_sync(sim, inst, &b->lamp_test);
    int blank = b->blank.pin == SEQ_NO_PIN ? 0 : seq_sync(sim, inst, &b->blank);
    unsigned known = 0, value = 0;
    size_t i, j;

    for (i = 0; i < 4; i++) {
        Merge m = {0, LOGIC_X};
        LogicValue d = d_value(sim, inst, b->inputs[i]);
        int k;

        for (k = lo(hold); k <= hi(hold); k++) {
            merge(&m, k ? s->code[i] : d);
        }
        s->code[i] = merged(&m);
        if (s->code[i] != LOGIC_X) {
            known |= 1u << i;
            value |= (s->code[i] == LOGIC_1 ? 1u : 0u) << i;
        }
    }
    for (j = 0; j < 7; j++) {
        Merge m = {0, LOGIC_X};
        unsigned code;
        int lt, bl;

        for (code = 0; code < 16; code++) {
            if ((code & known) != value) {
                continue;
            }
            for (lt = lo(lamp); lt <= hi(lamp); lt++) {
                for (bl = lo(blank); bl <= hi(blank); bl++) {
                    if (lt) {
                        merge(&m, LOGIC_1);
                    } else if (bl) {
                        merge(&m, LOGIC_0);
                    } else {
                        merge(&m, (b->glyphs[code] >> j) & 1u ? LOGIC_1 : LOGIC_0);
                    }
                }
            }
        }
        seq_drive(sim, inst, &s->seg[j], merged(&m), SEQ_PUSH_PULL, h->timing);
    }
}

/* ---- bus-mux -------------------------------------------------------------- */

static const char *const MUX_KEYS[] = {
    "a", "b1", "b2", "select", "latchEnables", "outputEnables", NULL
};

typedef struct {
    const char *const *a, *const *b1, *const *b2;
    const char *select;
    const char *le_b1, *le_b2, *le_a1, *le_a2;
    const char *oe_a, *oe_b1, *oe_b2;
} MuxDevice;

typedef struct {
    size_t a[MUX_CHANNELS], b1[MUX_CHANNELS], b2[MUX_CHANNELS];
    size_t select;
    SeqControl le_b1, le_b2, le_a1, le_a2;
    SeqControl oe_a, oe_b1, oe_b2;
} MuxBound;

typedef struct {
    LatchHeader h;
    const MuxBound *b;
    LogicValue l1b[MUX_CHANNELS], l2b[MUX_CHANNELS], la1b[MUX_CHANNELS], la2b[MUX_CHANNELS];
    SeqOutput a[MUX_CHANNELS], b1[MUX_CHANNELS], b2[MUX_CHANNELS];
} MuxState;

static const char *const MUX_LE_KEYS[] = {"b1", "b2", "a1", "a2", NULL};
static const char *const MUX_OE_KEYS[] = {"a", "b1", "b2", NULL};

/* An object of pin names with exactly the given keys; out[i] is keys[i]'s. */
static int mux_names(KindContext *ctx, const cJSON *json, const char *key,
                     const char *const *keys, const char **out) {
    const cJSON *item = kind_member(ctx, json, key, 1);
    size_t i;

    if (item == NULL) {
        return -1;
    }
    kind_push(ctx, "%s", key);
    if (kind_check_keys(ctx, item, keys, NULL) != 0) {
        return -1;
    }
    for (i = 0; keys[i] != NULL; i++) {
        if (kind_string(ctx, item, keys[i], &out[i]) != 0) {
            return -1;
        }
    }
    kind_pop(ctx);
    return 0;
}

static void *mux_load(KindContext *ctx, const cJSON *json, unsigned *units) {
    MuxDevice *dev = kind_alloc(ctx, sizeof(*dev));
    const char *le[4];
    const char *oe[3];
    size_t n;

    if (dev == NULL || seq_names(ctx, json, "a", MUX_CHANNELS, MUX_CHANNELS, &dev->a, &n) != 0 ||
        seq_names(ctx, json, "b1", MUX_CHANNELS, MUX_CHANNELS, &dev->b1, &n) != 0 ||
        seq_names(ctx, json, "b2", MUX_CHANNELS, MUX_CHANNELS, &dev->b2, &n) != 0 ||
        kind_string(ctx, json, "select", &dev->select) != 0 ||
        mux_names(ctx, json, "latchEnables", MUX_LE_KEYS, le) != 0 ||
        mux_names(ctx, json, "outputEnables", MUX_OE_KEYS, oe) != 0) {
        return NULL;
    }
    dev->le_b1 = le[0];
    dev->le_b2 = le[1];
    dev->le_a1 = le[2];
    dev->le_a2 = le[3];
    dev->oe_a = oe[0];
    dev->oe_b1 = oe[1];
    dev->oe_b2 = oe[2];
    *units = MUX_CHANNELS;
    return dev;
}

static void *mux_bind(KindContext *ctx, const void *raw, const PartPin *pins, size_t count) {
    const MuxDevice *dev = raw;
    MuxBound *b = kind_alloc(ctx, sizeof(*b));
    SeqControlName le_b1 = {dev->le_b1, 1, 0}, le_b2 = {dev->le_b2, 1, 0};
    SeqControlName le_a1 = {dev->le_a1, 1, 0}, le_a2 = {dev->le_a2, 1, 0};
    SeqControlName oe_a = {dev->oe_a, 0, 0}, oe_b1 = {dev->oe_b1, 0, 0}, oe_b2 = {dev->oe_b2, 0, 0};

    if (b == NULL || bind_names(ctx, pins, count, dev->a, MUX_CHANNELS, b->a) != 0 ||
        bind_names(ctx, pins, count, dev->b1, MUX_CHANNELS, b->b1) != 0 ||
        bind_names(ctx, pins, count, dev->b2, MUX_CHANNELS, b->b2) != 0 ||
        seq_bind(ctx, pins, count, dev->select, &b->select) != 0 ||
        seq_bind_control(ctx, pins, count, &le_b1, &b->le_b1) != 0 ||
        seq_bind_control(ctx, pins, count, &le_b2, &b->le_b2) != 0 ||
        seq_bind_control(ctx, pins, count, &le_a1, &b->le_a1) != 0 ||
        seq_bind_control(ctx, pins, count, &le_a2, &b->le_a2) != 0 ||
        seq_bind_control(ctx, pins, count, &oe_a, &b->oe_a) != 0 ||
        seq_bind_control(ctx, pins, count, &oe_b1, &b->oe_b1) != 0 ||
        seq_bind_control(ctx, pins, count, &oe_b2, &b->oe_b2) != 0) {
        return NULL;
    }
    return b;
}

static LatchHeader *mux_create(const void *dev, const void *bound) {
    const MuxBound *b = bound;
    MuxState *s = calloc(1, sizeof(*s));
    size_t i;

    (void)dev;
    if (s == NULL) {
        return NULL;
    }
    s->b = b;
    for (i = 0; i < MUX_CHANNELS; i++) {
        s->l1b[i] = s->l2b[i] = s->la1b[i] = s->la2b[i] = LOGIC_0;
    }
    init_outputs(s->a, b->a, MUX_CHANNELS);
    init_outputs(s->b1, b->b1, MUX_CHANNELS);
    init_outputs(s->b2, b->b2, MUX_CHANNELS);
    return &s->h;
}

/* A transparent latch with a high-active enable. */
static LogicValue mux_latch(LogicValue stored, int open, LogicValue d) {
    Merge m = {0, LOGIC_X};
    int e;

    for (e = lo(open); e <= hi(open); e++) {
        merge(&m, e ? d : stored);
    }
    return merged(&m);
}

static void mux_eval(Sim *sim, SimInstance *inst, LatchHeader *h, int at_start) {
    MuxState *s = (MuxState *)h;
    const MuxBound *b = s->b;
    int le_b1 = at_start ? 0 : seq_sync(sim, inst, &b->le_b1);
    int le_b2 = at_start ? 0 : seq_sync(sim, inst, &b->le_b2);
    int le_a1 = at_start ? 0 : seq_sync(sim, inst, &b->le_a1);
    int le_a2 = at_start ? 0 : seq_sync(sim, inst, &b->le_a2);
    int oe_a = seq_enable(sim, inst, &b->oe_a);
    int oe_b1 = seq_enable(sim, inst, &b->oe_b1), oe_b2 = seq_enable(sim, inst, &b->oe_b2);
    int sel = seq_read(sim, inst, b->select);
    size_t i;

    for (i = 0; i < MUX_CHANNELS; i++) {
        LogicValue a = d_value(sim, inst, b->a[i]);
        LogicValue a_out;

        s->l1b[i] = mux_latch(s->l1b[i], le_b1, d_value(sim, inst, b->b1[i]));
        s->l2b[i] = mux_latch(s->l2b[i], le_b2, d_value(sim, inst, b->b2[i]));
        s->la1b[i] = mux_latch(s->la1b[i], le_a1, a);
        s->la2b[i] = mux_latch(s->la2b[i], le_a2, a);
        if (sel == 1) {
            a_out = s->l1b[i];
        } else if (sel == 0) {
            a_out = s->l2b[i];
        } else {
            a_out = s->l1b[i] == s->l2b[i] ? s->l1b[i] : LOGIC_X;
        }
        seq_drive(sim, inst, &s->a[i], seq_gate(a_out, oe_a), SEQ_PUSH_PULL, h->timing);
        seq_drive(sim, inst, &s->b1[i], seq_gate(s->la1b[i], oe_b1), SEQ_PUSH_PULL, h->timing);
        seq_drive(sim, inst, &s->b2[i], seq_gate(s->la2b[i], oe_b2), SEQ_PUSH_PULL, h->timing);
    }
}

/* ---- The kind ------------------------------------------------------------- */

static const LatchModel MODELS[] = {
    {"transparent", TRANSPARENT_KEYS, tl_load, tl_bind, tl_create, tl_eval, tl_open_drain},
    {"addressable", ADDRESSABLE_KEYS, ad_load, ad_bind, ad_create, ad_eval, ad_open_drain},
    {"sr", SR_KEYS, sr_load, sr_bind, sr_create, sr_eval, NULL},
    {"bcd7seg", BCD_KEYS, bcd_load, bcd_bind, bcd_create, bcd_eval, NULL},
    {"bus-mux", MUX_KEYS, mux_load, mux_bind, mux_create, mux_eval, NULL},
};

#define MODEL_COUNT (sizeof(MODELS) / sizeof(MODELS[0]))

/* "model", "timing" and every model's keys (the library checks these first). */
static const char *const DEVICE_KEYS[] = {
    "model", "timing",
    "units", "enableActive", "polarity", "outputEnableActive", "presetActive", "clearActive",
    "presetWins", "output",
    "address", "data", "outputs", "enable", "clear", "invert",
    "inputsActive", "both", "outputEnable",
    "inputs", "latch", "lampTest", "blank", "segments", "glyphs",
    "a", "b1", "b2", "select", "latchEnables", "outputEnables",
    NULL
};
static const char *const PART_KEYS[] = {"family", "vcc", "inputMax", NULL};

static void *latch_load_device(KindContext *ctx, const cJSON *json) {
    LatchDevice *dev = kind_alloc(ctx, sizeof(*dev));
    const char *name;
    unsigned units = 0;
    size_t i;

    if (dev == NULL || kind_string(ctx, json, "model", &name) != 0) {
        return NULL;
    }
    for (i = 0; i < MODEL_COUNT && strcmp(MODELS[i].name, name) != 0; i++) {
    }
    if (i == MODEL_COUNT) {
        kind_error(ctx, "unknown model \"%s\"", name);
        return NULL;
    }
    dev->model = &MODELS[i];
    if (seq_check_model_keys(ctx, json, name, dev->model->keys) != 0) {
        return NULL;
    }
    dev->dev = dev->model->load(ctx, json, &units);
    if (dev->dev == NULL || seq_timing(ctx, json, NULL, units, &dev->timing) != 0) {
        return NULL;
    }
    return dev;
}

static void *latch_bind_pinout(KindContext *ctx, const void *raw, const PartPin *pins,
                               size_t pin_count) {
    const LatchDevice *dev = raw;

    return dev->model->bind(ctx, dev->dev, pins, pin_count);
}

static int latch_load_part(KindContext *ctx, const void *raw, const void *bound,
                           const cJSON *json, Part *out) {
    const LatchDevice *dev = raw;
    const SeqTiming *entry;

    (void)bound;
    if (seq_part(ctx, &dev->timing, json, out, &entry) != 0) {
        return -1;
    }
    kind_set_part_data(out, entry);
    return 0;
}

static int latch_pin_open_drain(const Part *part, size_t pin) {
    const PartInternal *internal = part_internal(part);
    const LatchDevice *dev = internal->device;

    return dev->model->open_drain != NULL &&
           dev->model->open_drain(dev->dev, internal->bound, pin);
}

/* ---- Engine callbacks ----------------------------------------------------- */

static size_t latch_pin_count(const void *config) {
    return ((const Part *)config)->pin_count;
}

static void *latch_create(const void *config) {
    const PartInternal *internal = part_internal(config);
    const LatchDevice *dev = internal->device;
    LatchHeader *h = dev->model->create(dev->dev, internal->bound);

    if (h != NULL) {
        h->model = dev->model;
        h->timing = internal->data;
    }
    return h;
}

static void latch_destroy(void *state) {
    free(state);
}

static void latch_start(Sim *sim, SimInstance *instance, void *state) {
    LatchHeader *h = state;

    h->model->eval(sim, instance, h, 1);
}

static void latch_pin_changed(Sim *sim, SimInstance *instance, void *state, size_t pin,
                              LogicValue old_value, LogicValue new_value) {
    LatchHeader *h = state;

    (void)pin;
    (void)old_value;
    (void)new_value;
    h->model->eval(sim, instance, h, 0);
}

static const SimComponentType LATCH_MODEL = {
    .name = "latch",
    .pin_count = latch_pin_count,
    .create = latch_create,
    .destroy = latch_destroy,
    .start = latch_start,
    .pin_changed = latch_pin_changed,
};

const PartKindOps LATCH_KIND = {
    PART_LATCH, "latch", &LATCH_MODEL, DEVICE_KEYS, PART_KEYS,
    latch_load_device, latch_bind_pinout, latch_load_part, latch_pin_open_drain
};
