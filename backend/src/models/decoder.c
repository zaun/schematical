/*
 * decoder: combinational decoders and encoders (plan-19-B). One kind, three
 * models, picked by the device's "model" key:
 *
 *   binary     n-to-2^n decoders / demultiplexers, BCD-to-decimal decoders
 *              (74x138, 74x139, 74x42, 4028B, 4514B, 74x131, SN74BCT2414 ...)
 *   priority   priority encoders (74x147, 74x148, 74x348, 4532B, 40147B)
 *   seg7       BCD-to-7-segment decoders/drivers (SN7447A, SN74LS49, CD4056B)
 *
 * Every model re-evaluates all of its outputs on any pin change. An address
 * latch or register (binary) and an input latch (seg7) hold 0 at power-on.
 * Unknown inputs are handled by trying every level of each unknown input: an
 * output whose outcomes differ becomes X. Signal rules: docs/parts-format.md
 * "Sequential kinds" and "decoder".
 */

#include <stdlib.h>
#include <string.h>

#include "kind.h"
#include "model_pins.h"
#include "seq.h"

#define DEC_MAX_UNITS 4
#define DEC_MAX_ADDRESS 4
#define DEC_MAX_OUTPUTS 16
#define DEC_MAX_ENABLES 3
#define PRI_MAX_INPUTS 10
#define PRI_MAX_CODE 4

/* ---- Model table ---------------------------------------------------------- */

typedef struct DecoderModel DecoderModel;

/* Every model's run state starts with this. */
typedef struct {
    const DecoderModel *model;
    const SeqTiming *timing;
} DecoderHeader;

struct DecoderModel {
    const char *name;
    const char *const *keys;
    void *(*load)(KindContext *ctx, const cJSON *json, unsigned *units);
    void *(*bind)(KindContext *ctx, const void *dev, const PartPin *pins, size_t count);
    DecoderHeader *(*create)(const void *bound);
    /* pin: the pin that changed (SEQ_NO_PIN at start), with its old and new value. */
    void (*eval)(Sim *sim, SimInstance *inst, DecoderHeader *state, size_t pin,
                 LogicValue old_value, LogicValue new_value);
    int (*open_drain)(const void *bound, size_t pin);
};

typedef struct {
    const DecoderModel *model;
    void *dev;
    SeqTimingTable timing;
} DecoderDevice;

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

static int lo(int level) {
    return level < 0 ? 0 : level;
}

static int hi(int level) {
    return level < 0 ? 1 : level;
}

static LogicValue level_value(int level) {
    return level ? LOGIC_1 : LOGIC_0;
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

static int pin_in(size_t pin, const size_t *pins, size_t n) {
    size_t i;

    for (i = 0; i < n; i++) {
        if (pins[i] == pin) {
            return 1;
        }
    }
    return 0;
}

/* "inactive" (0) or "z" (1) at json[key]; absent -> 0. */
static int inactive_or_z(KindContext *ctx, const cJSON *json, const char *key, int *z) {
    const char *text;

    *z = 0;
    if (cJSON_GetObjectItemCaseSensitive(json, key) == NULL) {
        return 0;
    }
    if (kind_string(ctx, json, key, &text) != 0) {
        return -1;
    }
    if (strcmp(text, "z") == 0) {
        *z = 1;
    } else if (strcmp(text, "inactive") != 0) {
        return kind_error(ctx, "\"%s\" must be \"inactive\" or \"z\", not \"%s\"", key, text);
    }
    return 0;
}

/* Known bits and their values of the given levels (1, 0, -1). */
static void known_bits(const int *levels, size_t n, unsigned *known, unsigned *value) {
    size_t i;

    *known = 0;
    *value = 0;
    for (i = 0; i < n; i++) {
        if (levels[i] >= 0) {
            *known |= 1u << i;
            *value |= (unsigned)levels[i] << i;
        }
    }
}

/* ---- binary --------------------------------------------------------------- */

static const char *const BINARY_KEYS[] = {
    "units", "outputsActive", "unselected", "disabled", "latchActive", "output", "monitor", NULL
};

typedef struct {
    const char *const *address;
    size_t address_count;
    const char *const *outputs;
    size_t output_count;
    SeqControlName enables[DEC_MAX_ENABLES];
    size_t enable_count;
    const char *latch, *clock;
} BinUnitNames;

typedef struct {
    BinUnitNames units[DEC_MAX_UNITS];
    size_t unit_count;
    int outputs_high, unselected_z, disabled_z, latch_hold_high;
    SeqOutputStyle style;
    const char *monitor;
} BinDevice;

typedef struct {
    size_t address[DEC_MAX_ADDRESS];
    size_t address_count;
    size_t outputs[DEC_MAX_OUTPUTS];
    size_t output_count;
    SeqControl enables[DEC_MAX_ENABLES];
    size_t enable_count;
    SeqControl latch;   /* asserted = hold */
    size_t clock;
} BinUnit;

typedef struct {
    BinUnit units[DEC_MAX_UNITS];
    size_t unit_count;
    int outputs_high, unselected_z, disabled_z;
    SeqOutputStyle style;
    size_t monitor;
} BinBound;

typedef struct {
    DecoderHeader h;
    const BinBound *b;
    LogicValue stored[DEC_MAX_UNITS][DEC_MAX_ADDRESS];
    SeqOutput out[DEC_MAX_UNITS][DEC_MAX_OUTPUTS];
    SeqOutput monitor;
} BinState;

static int bin_load_unit(KindContext *ctx, const cJSON *item, BinUnitNames *u) {
    static const char *const UNIT_KEYS[] = {"address", "outputs", "enables", "latch", "clock", NULL};
    const cJSON *enables;

    if (kind_check_keys(ctx, item, UNIT_KEYS, NULL) != 0 ||
        seq_names(ctx, item, "address", 1, DEC_MAX_ADDRESS, &u->address, &u->address_count) != 0 ||
        seq_names(ctx, item, "outputs", 1, DEC_MAX_OUTPUTS, &u->outputs, &u->output_count) != 0 ||
        seq_optional_name(ctx, item, "latch", &u->latch) != 0 ||
        seq_optional_name(ctx, item, "clock", &u->clock) != 0) {
        return -1;
    }
    if (u->output_count > ((size_t)1 << u->address_count)) {
        return kind_error(ctx, "\"outputs\" must have at most %zu names (2 to the number of "
                          "address pins)", (size_t)1 << u->address_count);
    }
    if (u->latch != NULL && u->clock != NULL) {
        return kind_error(ctx, "a unit cannot have both \"latch\" and \"clock\"");
    }
    enables = cJSON_GetObjectItemCaseSensitive(item, "enables");
    u->enable_count = 0;
    if (enables != NULL) {
        const cJSON *e;

        if (!cJSON_IsArray(enables) || cJSON_GetArraySize(enables) > DEC_MAX_ENABLES) {
            return kind_error(ctx, "\"enables\" must be an array of 0 to %d controls",
                              DEC_MAX_ENABLES);
        }
        cJSON_ArrayForEach(e, enables) {
            kind_push(ctx, "enables[%zu]", u->enable_count);
            if (seq_control_object(ctx, e, 0, &u->enables[u->enable_count]) != 0) {
                return -1;
            }
            kind_pop(ctx);
            u->enable_count++;
        }
    }
    return 0;
}

static void *bin_load(KindContext *ctx, const cJSON *json, unsigned *units_out) {
    BinDevice *dev = kind_alloc(ctx, sizeof(*dev));
    const cJSON *units = kind_member(ctx, json, "units", 1);
    const cJSON *item;
    int any_latch = 0;
    size_t n = 0;

    if (dev == NULL || units == NULL) {
        return NULL;
    }
    if (!cJSON_IsArray(units) || cJSON_GetArraySize(units) < 1 ||
        cJSON_GetArraySize(units) > DEC_MAX_UNITS) {
        kind_error(ctx, "\"units\" must be an array of 1 to %d units", DEC_MAX_UNITS);
        return NULL;
    }
    cJSON_ArrayForEach(item, units) {
        kind_push(ctx, "units[%zu]", n);
        if (bin_load_unit(ctx, item, &dev->units[n]) != 0) {
            return NULL;
        }
        any_latch |= dev->units[n].latch != NULL;
        kind_pop(ctx);
        n++;
    }
    dev->unit_count = n;
    if (seq_active(ctx, json, "outputsActive", &dev->outputs_high) != 0 ||
        inactive_or_z(ctx, json, "unselected", &dev->unselected_z) != 0 ||
        inactive_or_z(ctx, json, "disabled", &dev->disabled_z) != 0 ||
        seq_output_style(ctx, json, &dev->style) != 0 ||
        seq_optional_name(ctx, json, "monitor", &dev->monitor) != 0) {
        return NULL;
    }
    if (any_latch) {
        if (seq_active(ctx, json, "latchActive", &dev->latch_hold_high) != 0) {
            return NULL;
        }
    } else if (cJSON_GetObjectItemCaseSensitive(json, "latchActive") != NULL) {
        kind_error(ctx, "\"latchActive\" needs a unit with \"latch\"");
        return NULL;
    }
    *units_out = (unsigned)n;
    return dev;
}

static void *bin_bind(KindContext *ctx, const void *raw, const PartPin *pins, size_t count) {
    const BinDevice *dev = raw;
    BinBound *b = kind_alloc(ctx, sizeof(*b));
    size_t i, j;

    if (b == NULL) {
        return NULL;
    }
    for (i = 0; i < dev->unit_count; i++) {
        const BinUnitNames *n = &dev->units[i];
        BinUnit *u = &b->units[i];
        SeqControlName latch = {n->latch, dev->latch_hold_high, 0};

        kind_push(ctx, "units[%zu]", i);
        if (bind_names(ctx, pins, count, n->address, n->address_count, u->address) != 0 ||
            bind_names(ctx, pins, count, n->outputs, n->output_count, u->outputs) != 0 ||
            seq_bind_control(ctx, pins, count, &latch, &u->latch) != 0 ||
            seq_bind(ctx, pins, count, n->clock, &u->clock) != 0) {
            return NULL;
        }
        for (j = 0; j < n->enable_count; j++) {
            if (seq_bind_control(ctx, pins, count, &n->enables[j], &u->enables[j]) != 0) {
                return NULL;
            }
        }
        u->address_count = n->address_count;
        u->output_count = n->output_count;
        u->enable_count = n->enable_count;
        kind_pop(ctx);
    }
    if (seq_bind(ctx, pins, count, dev->monitor, &b->monitor) != 0) {
        return NULL;
    }
    b->unit_count = dev->unit_count;
    b->outputs_high = dev->outputs_high;
    b->unselected_z = dev->unselected_z;
    b->disabled_z = dev->disabled_z;
    b->style = dev->style;
    return b;
}

static DecoderHeader *bin_create(const void *bound) {
    const BinBound *b = bound;
    BinState *s = calloc(1, sizeof(*s));
    size_t i, j;

    if (s == NULL) {
        return NULL;
    }
    s->b = b;
    for (i = 0; i < b->unit_count; i++) {
        for (j = 0; j < DEC_MAX_ADDRESS; j++) {
            s->stored[i][j] = LOGIC_0;
        }
        init_outputs(s->out[i], b->units[i].outputs, b->units[i].output_count);
    }
    init_outputs(&s->monitor, &b->monitor, 1);
    return &s->h;
}

/* The unit's enable: 1 every enable asserted, 0 one deasserted, -1 unknown. */
static int bin_enabled(const Sim *sim, const SimInstance *inst, const BinUnit *u) {
    int result = 1;
    size_t j;

    for (j = 0; j < u->enable_count; j++) {
        int level = seq_sync(sim, inst, &u->enables[j]);

        if (level == 0) {
            return 0;
        }
        if (level < 0) {
            result = -1;
        }
    }
    return result;
}

static void bin_eval(Sim *sim, SimInstance *inst, DecoderHeader *h, size_t pin,
                     LogicValue old_value, LogicValue new_value) {
    BinState *s = (BinState *)h;
    const BinBound *b = s->b;
    const LogicValue active = level_value(b->outputs_high);
    const LogicValue inactive = level_value(!b->outputs_high);
    size_t i, j;

    for (i = 0; i < b->unit_count; i++) {
        const BinUnit *u = &b->units[i];
        int enabled = bin_enabled(sim, inst, u);
        int levels[DEC_MAX_ADDRESS];
        unsigned known, value, code;
        unsigned codes = 1u << u->address_count;

        if (u->clock != SEQ_NO_PIN) {
            if (pin == u->clock && seq_rising(old_value, new_value)) {
                for (j = 0; j < u->address_count; j++) {
                    s->stored[i][j] = seq_value(seq_read(sim, inst, u->address[j]));
                }
            }
        } else if (u->latch.pin != SEQ_NO_PIN) {
            int hold = pin == SEQ_NO_PIN ? 1 : seq_sync(sim, inst, &u->latch);

            for (j = 0; j < u->address_count; j++) {
                Merge m = {0, LOGIC_X};
                int k;

                for (k = lo(hold); k <= hi(hold); k++) {
                    merge(&m, k ? s->stored[i][j]
                                : seq_value(seq_read(sim, inst, u->address[j])));
                }
                s->stored[i][j] = merged(&m);
            }
        } else {
            for (j = 0; j < u->address_count; j++) {
                s->stored[i][j] = seq_value(seq_read(sim, inst, u->address[j]));
            }
        }
        for (j = 0; j < u->address_count; j++) {
            levels[j] = seq_level(s->stored[i][j]);
        }
        known_bits(levels, u->address_count, &known, &value);
        for (j = 0; j < u->output_count; j++) {
            Merge m = {0, LOGIC_X};

            for (code = 0; code < codes; code++) {
                if ((code & known) != value) {
                    continue;
                }
                if (code == j) {
                    /* Selected: active when enabled; an X enable makes it X. */
                    merge(&m, enabled < 0 ? LOGIC_X
                              : enabled  ? active
                              : b->disabled_z ? LOGIC_Z : inactive);
                } else if (b->unselected_z) {
                    merge(&m, LOGIC_Z);
                } else {
                    /* An X enable makes every output it gates X. */
                    merge(&m, enabled < 0 ? LOGIC_X
                              : !enabled && b->disabled_z ? LOGIC_Z : inactive);
                }
            }
            seq_drive(sim, inst, &s->out[i][j], merged(&m), b->style, h->timing);
        }
    }
    /* The supply monitor (SN74BCT2414 VS, open collector): low while the
       supply is good, which it always is in the engine. */
    seq_drive(sim, inst, &s->monitor, LOGIC_0, SEQ_OPEN_DRAIN, h->timing);
}

static int bin_open_drain(const void *bound, size_t pin) {
    const BinBound *b = bound;
    size_t i;

    if (b->monitor != SEQ_NO_PIN && pin == b->monitor) {
        return 1;
    }
    for (i = 0; b->style == SEQ_OPEN_DRAIN && i < b->unit_count; i++) {
        if (pin_in(pin, b->units[i].outputs, b->units[i].output_count)) {
            return 1;
        }
    }
    return 0;
}

/* ---- priority ------------------------------------------------------------- */

static const char *const PRIORITY_KEYS[] = {
    "inputs", "inputsActive", "code", "codeActive", "offset", "idle", "enableIn",
    "groupSelect", "enableOut", "disabled", "output", NULL
};

typedef struct {
    const char *const *inputs;
    size_t input_count;
    const char *const *code;
    size_t code_count;
    int inputs_high, code_high;
    unsigned offset, idle;
    SeqControlName enable_in, group_select, enable_out;
    int disabled_z;
    SeqOutputStyle style;
} PriDevice;

typedef struct {
    size_t inputs[PRI_MAX_INPUTS];
    size_t input_count;
    size_t code[PRI_MAX_CODE];
    size_t code_count;
    int inputs_high, code_high;
    unsigned offset, idle;
    SeqControl enable_in, group_select, enable_out;
    int disabled_z;
    SeqOutputStyle style;
} PriBound;

typedef struct {
    DecoderHeader h;
    const PriBound *b;
    SeqOutput code[PRI_MAX_CODE];
    SeqOutput group_select, enable_out;
} PriState;

static int optional_uint(KindContext *ctx, const cJSON *json, const char *key, uint64_t max,
                         unsigned *out) {
    uint64_t v;

    *out = 0;
    if (cJSON_GetObjectItemCaseSensitive(json, key) == NULL) {
        return 0;
    }
    if (kind_uint(ctx, json, key, max, &v) != 0) {
        return -1;
    }
    *out = (unsigned)v;
    return 0;
}

static void *pri_load(KindContext *ctx, const cJSON *json, unsigned *units) {
    PriDevice *dev = kind_alloc(ctx, sizeof(*dev));
    unsigned codes;

    if (dev == NULL ||
        seq_names(ctx, json, "inputs", 4, PRI_MAX_INPUTS, &dev->inputs, &dev->input_count) != 0 ||
        seq_active(ctx, json, "inputsActive", &dev->inputs_high) != 0 ||
        seq_names(ctx, json, "code", 2, PRI_MAX_CODE, &dev->code, &dev->code_count) != 0 ||
        seq_active(ctx, json, "codeActive", &dev->code_high) != 0 ||
        optional_uint(ctx, json, "offset", 15, &dev->offset) != 0 ||
        optional_uint(ctx, json, "idle", 15, &dev->idle) != 0 ||
        seq_control_name(ctx, json, "enableIn", 0, 0, &dev->enable_in) != 0 ||
        seq_control_name(ctx, json, "groupSelect", 0, 0, &dev->group_select) != 0 ||
        seq_control_name(ctx, json, "enableOut", 0, 0, &dev->enable_out) != 0 ||
        inactive_or_z(ctx, json, "disabled", &dev->disabled_z) != 0 ||
        seq_output_style(ctx, json, &dev->style) != 0) {
        return NULL;
    }
    codes = 1u << dev->code_count;
    if (dev->offset + dev->input_count > codes) {
        kind_error(ctx, "\"offset\" plus the number of inputs must be at most %u (2 to the "
                   "number of code pins)", codes);
        return NULL;
    }
    if (dev->idle >= codes) {
        kind_error(ctx, "\"idle\" must be below %u (2 to the number of code pins)", codes);
        return NULL;
    }
    *units = 1;
    return dev;
}

static void *pri_bind(KindContext *ctx, const void *raw, const PartPin *pins, size_t count) {
    const PriDevice *dev = raw;
    PriBound *b = kind_alloc(ctx, sizeof(*b));

    if (b == NULL ||
        bind_names(ctx, pins, count, dev->inputs, dev->input_count, b->inputs) != 0 ||
        bind_names(ctx, pins, count, dev->code, dev->code_count, b->code) != 0 ||
        seq_bind_control(ctx, pins, count, &dev->enable_in, &b->enable_in) != 0 ||
        seq_bind_control(ctx, pins, count, &dev->group_select, &b->group_select) != 0 ||
        seq_bind_control(ctx, pins, count, &dev->enable_out, &b->enable_out) != 0) {
        return NULL;
    }
    b->input_count = dev->input_count;
    b->code_count = dev->code_count;
    b->inputs_high = dev->inputs_high;
    b->code_high = dev->code_high;
    b->offset = dev->offset;
    b->idle = dev->idle;
    b->disabled_z = dev->disabled_z;
    b->style = dev->style;
    return b;
}

static DecoderHeader *pri_create(const void *bound) {
    const PriBound *b = bound;
    PriState *s = calloc(1, sizeof(*s));

    if (s == NULL) {
        return NULL;
    }
    s->b = b;
    init_outputs(s->code, b->code, b->code_count);
    init_outputs(&s->group_select, &b->group_select.pin, 1);
    init_outputs(&s->enable_out, &b->enable_out.pin, 1);
    return &s->h;
}

static void pri_eval(Sim *sim, SimInstance *inst, DecoderHeader *h, size_t pin,
                     LogicValue old_value, LogicValue new_value) {
    PriState *s = (PriState *)h;
    const PriBound *b = s->b;
    int enabled = b->enable_in.pin == SEQ_NO_PIN ? 1 : seq_sync(sim, inst, &b->enable_in);
    int levels[PRI_MAX_INPUTS];
    size_t unknown[PRI_MAX_INPUTS];
    size_t unknown_count = 0;
    Merge code[PRI_MAX_CODE], gs = {0, LOGIC_X}, eo = {0, LOGIC_X};
    unsigned assignment;
    size_t i;

    (void)pin;
    (void)old_value;
    (void)new_value;
    for (i = 0; i < b->code_count; i++) {
        code[i].have = 0;
        code[i].value = LOGIC_X;
    }
    for (i = 0; i < b->input_count; i++) {
        int level = seq_read(sim, inst, b->inputs[i]);

        levels[i] = level < 0 ? -1 : level == b->inputs_high;   /* 1 = active */
        if (levels[i] < 0) {
            unknown[unknown_count++] = i;
        }
    }
    /* An X enable makes every output X: the merges stay empty. */
    for (assignment = 0; enabled >= 0 && assignment < (1u << unknown_count); assignment++) {
        int active[PRI_MAX_INPUTS];
        int winner = -1;
        unsigned value = b->idle;
        int group = 0, empty = 0, code_z = 0;

        for (i = 0; i < b->input_count; i++) {
            active[i] = levels[i];
        }
        for (i = 0; i < unknown_count; i++) {
            active[unknown[i]] = (int)((assignment >> i) & 1u);
        }
        for (i = 0; i < b->input_count; i++) {
            if (active[i]) {
                winner = (int)i;
            }
        }
        if (!enabled) {
            code_z = b->disabled_z;
        } else if (winner >= 0) {
            value = (unsigned)winner + b->offset;
            group = 1;
        } else {
            empty = 1;
            code_z = b->disabled_z;
        }
        for (i = 0; i < b->code_count; i++) {
            int bit = (int)((value >> i) & 1u);

            merge(&code[i], code_z ? LOGIC_Z : level_value(b->code_high ? bit : !bit));
        }
        merge(&gs, level_value(group == b->group_select.active_high));
        merge(&eo, level_value(empty == b->enable_out.active_high));
    }
    for (i = 0; i < b->code_count; i++) {
        seq_drive(sim, inst, &s->code[i], merged(&code[i]), b->style, h->timing);
    }
    seq_drive(sim, inst, &s->group_select, merged(&gs), b->style, h->timing);
    seq_drive(sim, inst, &s->enable_out, merged(&eo), b->style, h->timing);
}

static int pri_open_drain(const void *bound, size_t pin) {
    const PriBound *b = bound;

    return b->style == SEQ_OPEN_DRAIN &&
           (pin_in(pin, b->code, b->code_count) || pin == b->group_select.pin ||
            pin == b->enable_out.pin);
}

/* ---- seg7 ----------------------------------------------------------------- */

static const char *const SEG7_KEYS[] = {
    "inputs", "segments", "segmentsActive", "glyphs", "lampTest", "blank", "rbi", "rbo",
    "latch", "phase", "output", NULL
};

typedef struct {
    const char *const *inputs;
    size_t input_count;
    const char *const *segments;
    size_t segment_count;
    int segments_high;
    unsigned char glyphs[16];   /* bit j = segment j (a = bit 0) */
    SeqControlName lamp_test, blank, latch;
    const char *rbi, *rbo, *phase;
    SeqOutputStyle style;
} SegDevice;

typedef struct {
    size_t inputs[4];
    size_t segments[7];
    int segments_high;
    const unsigned char *glyphs;
    SeqControl lamp_test, blank, latch;
    size_t rbi, rbo, phase;
    SeqOutputStyle style;
} SegBound;

typedef struct {
    DecoderHeader h;
    const SegBound *b;
    LogicValue code[4];
    SeqOutput seg[7];
    SeqOutput rbo;
} SegState;

static void *seg_load(KindContext *ctx, const cJSON *json, unsigned *units) {
    SegDevice *dev = kind_alloc(ctx, sizeof(*dev));
    const char *const *glyphs;
    size_t glyph_count;
    size_t i, j;

    if (dev == NULL ||
        seq_names(ctx, json, "inputs", 4, 4, &dev->inputs, &dev->input_count) != 0 ||
        seq_names(ctx, json, "segments", 7, 7, &dev->segments, &dev->segment_count) != 0 ||
        seq_active(ctx, json, "segmentsActive", &dev->segments_high) != 0 ||
        seq_names(ctx, json, "glyphs", 16, 16, &glyphs, &glyph_count) != 0 ||
        seq_control_name(ctx, json, "lampTest", 0, 0, &dev->lamp_test) != 0 ||
        seq_control_name(ctx, json, "blank", 0, 0, &dev->blank) != 0 ||
        seq_control_name(ctx, json, "latch", 0, 0, &dev->latch) != 0 ||
        seq_optional_name(ctx, json, "rbi", &dev->rbi) != 0 ||
        seq_optional_name(ctx, json, "rbo", &dev->rbo) != 0 ||
        seq_optional_name(ctx, json, "phase", &dev->phase) != 0 ||
        seq_output_style(ctx, json, &dev->style) != 0) {
        return NULL;
    }
    for (i = 0; i < 16; i++) {
        int ok = strlen(glyphs[i]) == 7;

        for (j = 0; ok && j < 7; j++) {
            ok = glyphs[i][j] == '0' || glyphs[i][j] == '1';
            if (glyphs[i][j] == '1') {
                dev->glyphs[i] |= (unsigned char)(1u << j);
            }
        }
        if (!ok) {
            kind_error(ctx, "glyphs[%zu]: must be 7 characters 0 or 1 (segments a to g)", i);
            return NULL;
        }
    }
    if ((dev->rbi != NULL) != (dev->rbo != NULL)) {
        kind_error(ctx, "\"rbi\" and \"rbo\" go together");
        return NULL;
    }
    *units = 1;
    return dev;
}

static void *seg_bind(KindContext *ctx, const void *raw, const PartPin *pins, size_t count) {
    const SegDevice *dev = raw;
    SegBound *b = kind_alloc(ctx, sizeof(*b));

    if (b == NULL || bind_names(ctx, pins, count, dev->inputs, 4, b->inputs) != 0 ||
        bind_names(ctx, pins, count, dev->segments, 7, b->segments) != 0 ||
        seq_bind_control(ctx, pins, count, &dev->lamp_test, &b->lamp_test) != 0 ||
        seq_bind_control(ctx, pins, count, &dev->blank, &b->blank) != 0 ||
        seq_bind_control(ctx, pins, count, &dev->latch, &b->latch) != 0 ||
        seq_bind(ctx, pins, count, dev->rbi, &b->rbi) != 0 ||
        seq_bind(ctx, pins, count, dev->rbo, &b->rbo) != 0 ||
        seq_bind(ctx, pins, count, dev->phase, &b->phase) != 0) {
        return NULL;
    }
    if (b->rbo != SEQ_NO_PIN && pins[b->rbo].role != PIN_INOUT) {
        kind_error(ctx, "\"rbo\" pin \"%s\" must have role inout", dev->rbo);
        return NULL;
    }
    b->segments_high = dev->segments_high;
    b->glyphs = dev->glyphs;
    b->style = dev->style;
    return b;
}

static DecoderHeader *seg_create(const void *bound) {
    const SegBound *b = bound;
    SegState *s = calloc(1, sizeof(*s));
    size_t i;

    if (s == NULL) {
        return NULL;
    }
    s->b = b;
    for (i = 0; i < 4; i++) {
        s->code[i] = LOGIC_0;
    }
    init_outputs(s->seg, b->segments, 7);
    init_outputs(&s->rbo, &b->rbo, 1);
    return &s->h;
}

/* An optional control: no pin -> 0 (never asserted). */
static int seg_control(const Sim *sim, const SimInstance *inst, const SeqControl *c) {
    return c->pin == SEQ_NO_PIN ? 0 : seq_sync(sim, inst, c);
}

static void seg_eval(Sim *sim, SimInstance *inst, DecoderHeader *h, size_t pin,
                     LogicValue old_value, LogicValue new_value) {
    SegState *s = (SegState *)h;
    const SegBound *b = s->b;
    int lamp = seg_control(sim, inst, &b->lamp_test);
    int blank = seg_control(sim, inst, &b->blank);
    int rbi = 0, bi = 0, phase = 0;
    int levels[4];
    unsigned known, value;
    Merge ripple = {0, LOGIC_X};
    size_t i, j;

    (void)old_value;
    (void)new_value;
    if (b->latch.pin != SEQ_NO_PIN) {
        int hold = pin == SEQ_NO_PIN ? 1 : seq_sync(sim, inst, &b->latch);

        for (i = 0; i < 4; i++) {
            Merge m = {0, LOGIC_X};
            int k;

            for (k = lo(hold); k <= hi(hold); k++) {
                merge(&m, k ? s->code[i] : seq_value(seq_read(sim, inst, b->inputs[i])));
            }
            s->code[i] = merged(&m);
        }
    } else {
        for (i = 0; i < 4; i++) {
            s->code[i] = seq_value(seq_read(sim, inst, b->inputs[i]));
        }
    }
    for (i = 0; i < 4; i++) {
        levels[i] = seq_level(s->code[i]);
    }
    known_bits(levels, 4, &known, &value);
    if (b->rbi != SEQ_NO_PIN) {
        int level = seq_read(sim, inst, b->rbi);
        LogicValue v = sim_read_pin(sim, inst, b->rbo);

        rbi = level < 0 ? -1 : !level;                                   /* RBI low */
        bi = v == LOGIC_0 ? 1 : v == LOGIC_X ? -1 : 0;                    /* BI/RBO low; Z reads high */
    }
    if (b->phase != SEQ_NO_PIN) {
        phase = seq_read(sim, inst, b->phase);
    }
    for (j = 0; j < 7; j++) {
        Merge m = {0, LOGIC_X};
        unsigned code;
        int lt, bl, ib, rb, ph;

        for (code = 0; code < 16; code++) {
            if ((code & known) != value) {
                continue;
            }
            for (lt = lo(lamp); lt <= hi(lamp); lt++) {
                for (bl = lo(blank); bl <= hi(blank); bl++) {
                    for (ib = lo(bi); ib <= hi(bi); ib++) {
                        for (rb = lo(rbi); rb <= hi(rbi); rb++) {
                            for (ph = lo(phase); ph <= hi(phase); ph++) {
                                int on;

                                /* blank (BI) > lamp test > ripple blank > glyph */
                                if (bl || ib) {
                                    on = 0;
                                } else if (lt) {
                                    on = 1;
                                } else if (rb && code == 0) {
                                    on = 0;
                                } else {
                                    on = (b->glyphs[code] >> j) & 1u;
                                }
                                merge(&m, level_value((on == b->segments_high) ^ ph));
                            }
                        }
                    }
                }
            }
        }
        seq_drive(sim, inst, &s->seg[j], merged(&m), b->style, h->timing);
    }
    if (b->rbo != SEQ_NO_PIN) {
        unsigned code;
        int lt, rb;

        /* RBO is pulled low while RBI is low, the code is 0 and lamp test is off. */
        for (code = 0; code < 16; code++) {
            if ((code & known) != value) {
                continue;
            }
            for (lt = lo(lamp); lt <= hi(lamp); lt++) {
                for (rb = lo(rbi); rb <= hi(rbi); rb++) {
                    merge(&ripple, rb && code == 0 && !lt ? LOGIC_0 : LOGIC_1);
                }
            }
        }
        seq_drive(sim, inst, &s->rbo, merged(&ripple), SEQ_OPEN_DRAIN, h->timing);
    }
}

static int seg_open_drain(const void *bound, size_t pin) {
    const SegBound *b = bound;

    return (b->style == SEQ_OPEN_DRAIN && pin_in(pin, b->segments, 7)) ||
           (b->rbo != SEQ_NO_PIN && pin == b->rbo);
}

/* ---- The kind ------------------------------------------------------------- */

static const DecoderModel MODELS[] = {
    {"binary", BINARY_KEYS, bin_load, bin_bind, bin_create, bin_eval, bin_open_drain},
    {"priority", PRIORITY_KEYS, pri_load, pri_bind, pri_create, pri_eval, pri_open_drain},
    {"seg7", SEG7_KEYS, seg_load, seg_bind, seg_create, seg_eval, seg_open_drain},
};

#define MODEL_COUNT (sizeof(MODELS) / sizeof(MODELS[0]))

/* "model", "timing" and every model's keys (the library checks these first). */
static const char *const DEVICE_KEYS[] = {
    "model", "timing", "output",
    "units", "outputsActive", "unselected", "disabled", "latchActive", "monitor",
    "inputs", "inputsActive", "code", "codeActive", "offset", "idle", "enableIn",
    "groupSelect", "enableOut",
    "segments", "segmentsActive", "glyphs", "lampTest", "blank", "rbi", "rbo", "latch", "phase",
    NULL
};
static const char *const PART_KEYS[] = {"family", "vcc", "inputMax", NULL};

static void *decoder_load_device(KindContext *ctx, const cJSON *json) {
    DecoderDevice *dev = kind_alloc(ctx, sizeof(*dev));
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

static void *decoder_bind_pinout(KindContext *ctx, const void *raw, const PartPin *pins,
                                 size_t pin_count) {
    const DecoderDevice *dev = raw;

    return dev->model->bind(ctx, dev->dev, pins, pin_count);
}

static int decoder_load_part(KindContext *ctx, const void *raw, const void *bound,
                             const cJSON *json, Part *out) {
    const DecoderDevice *dev = raw;
    const SeqTiming *entry;

    (void)bound;
    if (seq_part(ctx, &dev->timing, json, out, &entry) != 0) {
        return -1;
    }
    kind_set_part_data(out, entry);
    return 0;
}

static int decoder_pin_open_drain(const Part *part, size_t pin) {
    const PartInternal *internal = part_internal(part);
    const DecoderDevice *dev = internal->device;

    return dev->model->open_drain(internal->bound, pin);
}

/* ---- Engine callbacks ----------------------------------------------------- */

static size_t decoder_pin_count(const void *config) {
    return ((const Part *)config)->pin_count;
}

static void *decoder_create(const void *config) {
    const PartInternal *internal = part_internal(config);
    const DecoderDevice *dev = internal->device;
    DecoderHeader *h = dev->model->create(internal->bound);

    if (h != NULL) {
        h->model = dev->model;
        h->timing = internal->data;
    }
    return h;
}

static void decoder_destroy(void *state) {
    free(state);
}

static void decoder_start(Sim *sim, SimInstance *instance, void *state) {
    DecoderHeader *h = state;

    h->model->eval(sim, instance, h, SEQ_NO_PIN, LOGIC_X, LOGIC_X);
}

static void decoder_pin_changed(Sim *sim, SimInstance *instance, void *state, size_t pin,
                                LogicValue old_value, LogicValue new_value) {
    DecoderHeader *h = state;

    h->model->eval(sim, instance, h, pin, old_value, new_value);
}

static const SimComponentType DECODER_MODEL = {
    .name = "decoder",
    .pin_count = decoder_pin_count,
    .create = decoder_create,
    .destroy = decoder_destroy,
    .start = decoder_start,
    .pin_changed = decoder_pin_changed,
};

const PartKindOps DECODER_KIND = {
    PART_DECODER, "decoder", &DECODER_MODEL, DEVICE_KEYS, PART_KEYS,
    decoder_load_device, decoder_bind_pinout, decoder_load_part, decoder_pin_open_drain
};
