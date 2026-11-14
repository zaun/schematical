/*
 * register: multi-bit edge-triggered D registers (plan-37-C). One kind, one
 * model so far, picked by the device's "model" key:
 *
 *   parallel   groups of bits sharing a clock, an optional clock enable
 *              ("ce", 0-4 controls that must all be asserted), an optional
 *              clear (async, or acting at the clock edge) and one or two
 *              output banks, each optionally inverted and 3-state ("oe",
 *              0-4 controls). 74x173, 377-379, 534, 564, 575-577, 821-825,
 *              874, 11374, 29821, 4076B, 40174B, 16374 ... 16823.
 *
 * Per group, on any pin change: an asserted async clear sets every bit to 0
 * (X on it: every bit not already 0 becomes X); on the clock's edge a
 * clocked clear sets 0, else every bit loads D while all of "ce" is
 * asserted (Z on D stores X). Unknown controls or an edge from X/Z (or a
 * move into X/Z from the pre-edge level) make the bits the outcome would
 * change X. Each bank drives bit (or NOT bit) while all of its "oe" is
 * asserted, Z while one is deasserted, X while one is unknown and none is
 * deasserted. Power-on: every bit 0. Outputs are inertial: fall delay for
 * 0, rise for 1 and Z, the longer for X (seq_drive).
 * Signal rules: docs/parts-format.md "Sequential kinds", § register.
 */

#include <stdlib.h>
#include <string.h>

#include "kind.h"
#include "model_pins.h"
#include "seq.h"

#define REG_MAX_GROUPS 4
#define REG_MAX_BITS 40     /* per group */
#define REG_TOTAL_BITS 64   /* per device */
#define REG_MAX_CONTROLS 4
#define REG_MAX_BANKS 2

static const char *const PARALLEL_KEYS[] = {"groups", NULL};

typedef struct {
    SeqControlName names[REG_MAX_CONTROLS];
    size_t count;
} RegControlNames;

typedef struct {
    SeqControl controls[REG_MAX_CONTROLS];
    size_t count;
} RegControls;

typedef struct {
    const char *const *q;
    int invert;
    RegControlNames oe;
} RegBankDef;

typedef struct {
    const char *const *d;
    size_t bits;
    SeqClockName clk;
    RegControlNames ce;
    SeqControlName clear;   /* pin NULL = none */
    RegBankDef banks[REG_MAX_BANKS];
    size_t bank_count;
} RegGroupDef;

typedef struct {
    RegGroupDef groups[REG_MAX_GROUPS];
    size_t group_count;
    SeqTimingTable timing;
} RegDevice;

typedef struct {
    size_t q[REG_MAX_BITS];
    RegControls oe;
} RegBankBound;

typedef struct {
    size_t d[REG_MAX_BITS];
    SeqClock clk;
    RegControls ce;
    SeqControl clear;
    RegBankBound banks[REG_MAX_BANKS];
} RegGroupBound;

typedef struct {
    RegGroupBound groups[REG_MAX_GROUPS];
} RegBound;

/* ---- Loading -------------------------------------------------------------- */

/* "key": one control object or an array of 0-4; absent: none. */
static int load_controls(KindContext *ctx, const cJSON *json, const char *key,
                         RegControlNames *out) {
    const cJSON *item = cJSON_GetObjectItemCaseSensitive(json, key);
    const cJSON *e;
    size_t i = 0;

    out->count = 0;
    if (item == NULL) {
        return 0;
    }
    kind_push(ctx, "\"%s\"", key);
    if (cJSON_IsObject(item)) {
        if (seq_control_object(ctx, item, 0, &out->names[0]) != 0) {
            return -1;
        }
        out->count = 1;
    } else if (cJSON_IsArray(item)) {
        if (cJSON_GetArraySize(item) > REG_MAX_CONTROLS) {
            return kind_error(ctx, "at most %d controls", REG_MAX_CONTROLS);
        }
        cJSON_ArrayForEach(e, item) {
            kind_push(ctx, "[%zu]", i);
            if (seq_control_object(ctx, e, 0, &out->names[i]) != 0) {
                return -1;
            }
            kind_pop(ctx);
            i++;
        }
        out->count = i;
    } else {
        return kind_error(ctx, "must be a control or an array of controls");
    }
    kind_pop(ctx);
    return 0;
}

static int load_bank(KindContext *ctx, const cJSON *json, size_t bits, RegBankDef *bank) {
    static const char *const BANK_KEYS[] = {"q", "invert", "oe", NULL};
    size_t n;

    if (!cJSON_IsObject(json)) {
        return kind_error(ctx, "must be an object");
    }
    if (kind_check_keys(ctx, json, BANK_KEYS, NULL) != 0 ||
        kind_names(ctx, json, "q", &bank->q, &n) != 0) {
        return -1;
    }
    if (n != bits) {
        return kind_error(ctx, "\"q\" has %zu names, \"d\" has %zu", n, bits);
    }
    return seq_optional_bool(ctx, json, "invert", 0, &bank->invert) != 0 ||
                   load_controls(ctx, json, "oe", &bank->oe) != 0
               ? -1
               : 0;
}

static int load_group(KindContext *ctx, const cJSON *json, RegGroupDef *g) {
    static const char *const GROUP_KEYS[] = {"d", "clk", "ce", "clear", "outputs", NULL};
    const cJSON *outputs, *b;
    size_t i = 0;

    if (!cJSON_IsObject(json)) {
        return kind_error(ctx, "must be an object");
    }
    if (kind_check_keys(ctx, json, GROUP_KEYS, NULL) != 0 ||
        seq_names(ctx, json, "d", 1, REG_MAX_BITS, &g->d, &g->bits) != 0 ||
        seq_clock_name(ctx, json, "clk", 1, &g->clk) != 0 ||
        load_controls(ctx, json, "ce", &g->ce) != 0 ||
        seq_control_name(ctx, json, "clear", 0, 1, &g->clear) != 0) {
        return -1;
    }
    outputs = kind_member(ctx, json, "outputs", 1);
    if (outputs == NULL) {
        return -1;
    }
    if (!cJSON_IsArray(outputs) || cJSON_GetArraySize(outputs) < 1 ||
        cJSON_GetArraySize(outputs) > REG_MAX_BANKS) {
        return kind_error(ctx, "\"outputs\" must be an array of 1 to %d banks", REG_MAX_BANKS);
    }
    cJSON_ArrayForEach(b, outputs) {
        kind_push(ctx, "\"outputs\"[%zu]", i);
        if (load_bank(ctx, b, g->bits, &g->banks[i]) != 0) {
            return -1;
        }
        kind_pop(ctx);
        i++;
    }
    g->bank_count = i;
    return 0;
}

static void *parallel_load(KindContext *ctx, const cJSON *json, unsigned *units) {
    RegDevice *dev = kind_alloc(ctx, sizeof(*dev));
    const cJSON *groups = kind_member(ctx, json, "groups", 1);
    const cJSON *g;
    size_t i = 0, total = 0;

    if (dev == NULL || groups == NULL) {
        return NULL;
    }
    if (!cJSON_IsArray(groups) || cJSON_GetArraySize(groups) < 1 ||
        cJSON_GetArraySize(groups) > REG_MAX_GROUPS) {
        kind_error(ctx, "\"groups\" must be an array of 1 to %d groups", REG_MAX_GROUPS);
        return NULL;
    }
    cJSON_ArrayForEach(g, groups) {
        kind_push(ctx, "groups[%zu]", i);
        if (load_group(ctx, g, &dev->groups[i]) != 0) {
            return NULL;
        }
        kind_pop(ctx);
        total += dev->groups[i].bits;
        i++;
    }
    dev->group_count = i;
    if (total > REG_TOTAL_BITS) {
        kind_error(ctx, "at most %d stored bits per device, not %zu", REG_TOTAL_BITS, total);
        return NULL;
    }
    *units = (unsigned)total;
    return dev;
}

static int bind_controls(KindContext *ctx, const PartPin *pins, size_t count, const char *key,
                         const RegControlNames *names, RegControls *out) {
    size_t i;

    kind_push(ctx, "\"%s\"", key);
    out->count = names->count;
    for (i = 0; i < names->count; i++) {
        if (seq_bind_control(ctx, pins, count, &names->names[i], &out->controls[i]) != 0) {
            return -1;
        }
    }
    kind_pop(ctx);
    return 0;
}

static int bind_names(KindContext *ctx, const PartPin *pins, size_t count,
                      const char *const *names, size_t n, size_t *out) {
    size_t i;

    for (i = 0; i < n; i++) {
        if (kind_pin(ctx, pins, count, names[i], &out[i]) != 0) {
            return -1;
        }
    }
    return 0;
}

static void *parallel_bind(KindContext *ctx, const RegDevice *dev, const PartPin *pins,
                           size_t count) {
    RegBound *b = kind_alloc(ctx, sizeof(*b));
    size_t i, j;

    if (b == NULL) {
        return NULL;
    }
    for (i = 0; i < dev->group_count; i++) {
        const RegGroupDef *g = &dev->groups[i];
        RegGroupBound *gb = &b->groups[i];

        kind_push(ctx, "groups[%zu]", i);
        if (bind_names(ctx, pins, count, g->d, g->bits, gb->d) != 0 ||
            seq_bind_clock(ctx, pins, count, &g->clk, &gb->clk) != 0 ||
            bind_controls(ctx, pins, count, "ce", &g->ce, &gb->ce) != 0 ||
            seq_bind_control(ctx, pins, count, &g->clear, &gb->clear) != 0) {
            return NULL;
        }
        for (j = 0; j < g->bank_count; j++) {
            kind_push(ctx, "\"outputs\"[%zu]", j);
            if (bind_names(ctx, pins, count, g->banks[j].q, g->bits, gb->banks[j].q) != 0 ||
                bind_controls(ctx, pins, count, "oe", &g->banks[j].oe, &gb->banks[j].oe) != 0) {
                return NULL;
            }
            kind_pop(ctx);
        }
        kind_pop(ctx);
    }
    return b;
}

/* ---- The kind ------------------------------------------------------------- */

static const char *const DEVICE_KEYS[] = {"model", "timing", "groups", NULL};
static const char *const PART_KEYS[] = {"family", "vcc", "inputMax", NULL};

static void *register_load_device(KindContext *ctx, const cJSON *json) {
    RegDevice *dev;
    const char *name;
    unsigned units = 0;

    if (kind_string(ctx, json, "model", &name) != 0) {
        return NULL;
    }
    if (strcmp(name, "parallel") != 0) {
        kind_error(ctx, "unknown model \"%s\"", name);
        return NULL;
    }
    if (seq_check_model_keys(ctx, json, name, PARALLEL_KEYS) != 0) {
        return NULL;
    }
    dev = parallel_load(ctx, json, &units);
    if (dev == NULL || seq_timing(ctx, json, NULL, units, &dev->timing) != 0) {
        return NULL;
    }
    return dev;
}

static void *register_bind_pinout(KindContext *ctx, const void *raw, const PartPin *pins,
                                  size_t pin_count) {
    return parallel_bind(ctx, raw, pins, pin_count);
}

static int register_load_part(KindContext *ctx, const void *raw, const void *bound,
                              const cJSON *json, Part *out) {
    const RegDevice *dev = raw;
    const SeqTiming *entry;

    (void)bound;
    if (seq_part(ctx, &dev->timing, json, out, &entry) != 0) {
        return -1;
    }
    kind_set_part_data(out, entry);
    return 0;
}

/* ---- Running -------------------------------------------------------------- */

typedef struct {
    const RegDevice *dev;
    const RegBound *b;
    const SeqTiming *timing;
    SeqBits bits[REG_MAX_GROUPS];
    SeqOutput out[REG_MAX_GROUPS][REG_MAX_BANKS][REG_MAX_BITS];
} RegState;

/* All of the controls asserted: 1; one deasserted: 0; else -1. None: 1. */
static int all_asserted(const Sim *sim, const SimInstance *inst, const RegControls *c) {
    int result = 1;
    size_t i;

    for (i = 0; i < c->count; i++) {
        int v = seq_sync(sim, inst, &c->controls[i]);

        if (v == 0) {
            return 0;
        }
        if (v < 0) {
            result = -1;
        }
    }
    return result;
}

static LogicValue either(LogicValue a, LogicValue b) {
    return a == b ? a : LOGIC_X;
}

/* What a bit becomes if the edge happened: clr / ce are 1, 0 or -1. */
static LogicValue at_edge(LogicValue held, LogicValue d, int clr, int ce) {
    LogicValue loaded = ce == 1 ? d : ce == 0 ? held : either(d, held);

    if (clr == 1) {
        return LOGIC_0;
    }
    return clr == 0 ? loaded : either(LOGIC_0, loaded);
}

static void clock_group(Sim *sim, SimInstance *inst, RegState *s, size_t gi, int edge) {
    const RegGroupDef *g = &s->dev->groups[gi];
    const RegGroupBound *gb = &s->b->groups[gi];
    int clr = gb->clear.pin != SEQ_NO_PIN && gb->clear.clocked ? seq_sync(sim, inst, &gb->clear)
                                                              : 0;
    int ce = all_asserted(sim, inst, &gb->ce);
    unsigned i;

    for (i = 0; i < g->bits; i++) {
        LogicValue held = seq_bit(s->bits[gi], i);
        LogicValue d = seq_value(seq_read(sim, inst, gb->d[i]));
        LogicValue v = at_edge(held, d, clr, ce);

        seq_set_bit(&s->bits[gi], i, edge == 1 ? v : either(held, v));
    }
}

static void async_clear(Sim *sim, SimInstance *inst, RegState *s, size_t gi) {
    const RegGroupDef *g = &s->dev->groups[gi];
    const RegGroupBound *gb = &s->b->groups[gi];
    int c;
    unsigned i;

    if (gb->clear.pin == SEQ_NO_PIN || gb->clear.clocked) {
        return;
    }
    c = seq_async(sim, inst, &gb->clear);
    for (i = 0; c != 0 && i < g->bits; i++) {
        LogicValue v = seq_bit(s->bits[gi], i);

        seq_set_bit(&s->bits[gi], i, c == 1 ? LOGIC_0 : either(LOGIC_0, v));
    }
}

static void drive_group(Sim *sim, SimInstance *inst, RegState *s, size_t gi) {
    const RegGroupDef *g = &s->dev->groups[gi];
    const RegGroupBound *gb = &s->b->groups[gi];
    size_t k;
    unsigned i;

    for (k = 0; k < g->bank_count; k++) {
        int en = all_asserted(sim, inst, &gb->banks[k].oe);

        for (i = 0; i < g->bits; i++) {
            LogicValue v = seq_bit(s->bits[gi], i);

            if (g->banks[k].invert) {
                v = seq_not(v);
            }
            seq_drive(sim, inst, &s->out[gi][k][i], seq_gate(v, en), SEQ_PUSH_PULL, s->timing);
        }
    }
}

/* 1 the change was the clock's edge, -1 it may have been, 0 it was not. */
static int edge_of(const SeqClock *clk, LogicValue before, LogicValue now) {
    LogicValue from = clk->falling ? LOGIC_1 : LOGIC_0;

    if (before == now) {
        return 0;
    }
    if (seq_clock_edge(clk, before, now)) {
        return before == from ? 1 : -1;
    }
    if ((now == LOGIC_X || now == LOGIC_Z) && before == from) {
        return -1;
    }
    return 0;
}

static size_t register_pin_count(const void *config) {
    return ((const Part *)config)->pin_count;
}

static void *register_create(const void *config) {
    const PartInternal *internal = part_internal(config);
    RegState *s = calloc(1, sizeof(*s));
    size_t gi, k, i;

    if (s == NULL) {
        return NULL;
    }
    s->dev = internal->device;
    s->b = internal->bound;
    s->timing = internal->data;
    for (gi = 0; gi < s->dev->group_count; gi++) {
        const RegGroupDef *g = &s->dev->groups[gi];

        s->bits[gi].value = 0;
        s->bits[gi].known = g->bits >= 64 ? ~(uint64_t)0 : (((uint64_t)1 << g->bits) - 1);
        for (k = 0; k < g->bank_count; k++) {
            for (i = 0; i < g->bits; i++) {
                s->out[gi][k][i].pin = s->b->groups[gi].banks[k].q[i];
                s->out[gi][k][i].scheduled = 0;
                s->out[gi][k][i].driven = LOGIC_X;
            }
        }
    }
    return s;
}

static void register_destroy(void *state) {
    free(state);
}

static void register_start(Sim *sim, SimInstance *inst, void *state) {
    RegState *s = state;
    size_t gi;

    for (gi = 0; gi < s->dev->group_count; gi++) {
        async_clear(sim, inst, s, gi);
        drive_group(sim, inst, s, gi);
    }
}

static void register_pin_changed(Sim *sim, SimInstance *inst, void *state, size_t pin,
                                 LogicValue old_value, LogicValue new_value) {
    RegState *s = state;
    size_t gi;

    for (gi = 0; gi < s->dev->group_count; gi++) {
        const RegGroupBound *gb = &s->b->groups[gi];

        if (pin == gb->clk.pin) {
            int edge = edge_of(&gb->clk, old_value, new_value);

            if (edge != 0) {
                clock_group(sim, inst, s, gi, edge);
            }
        }
        async_clear(sim, inst, s, gi);
        drive_group(sim, inst, s, gi);
    }
}

static const SimComponentType REGISTER_MODEL = {
    .name = "register",
    .pin_count = register_pin_count,
    .create = register_create,
    .destroy = register_destroy,
    .start = register_start,
    .pin_changed = register_pin_changed,
};

const PartKindOps REGISTER_KIND = {
    PART_REGISTER, "register", &REGISTER_MODEL, DEVICE_KEYS, PART_KEYS,
    register_load_device, register_bind_pinout, register_load_part, NULL
};
