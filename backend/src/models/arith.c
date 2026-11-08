/*
 * arith: comparators, adders, ALUs, parity generators and Wallace trees
 * (plan-22-B). One kind; the device's "model" key picks the model:
 *
 *   compare    magnitude and identity comparators: 74x85, 4063B, 4585B,
 *              '682, '684, '688, '518, '520, '521, '885
 *   parity     parity generators/checkers: 74x280, '286, 74ACT11286
 *   adder, alu181, wallace
 *              the '283 adder, the '181 ALU and the '275 Wallace tree
 *              (arith_alu.c, plan-22-C)
 *
 * Every model is combinational (the '885 has a latch on its P word) and
 * re-evaluates all of its outputs on any pin change. An unknown input bit
 * makes X exactly the outputs whose value depends on it (arith_eval_unknown).
 * Signal rules: docs/parts-format.md "arith".
 */

#include <stdlib.h>
#include <string.h>

#include "arith.h"
#include "kind.h"
#include "model_pins.h"
#include "seq.h"

typedef struct {
    const ArithModel *model;
    void *dev;
    SeqTimingTable timing;
} ArithDevice;

/* ---- Shared helpers -------------------------------------------------------- */

uint32_t arith_eval_unknown(ArithFn fn, const void *bound, uint32_t inputs, uint32_t known,
                            unsigned input_count, unsigned outputs, uint32_t *out_known) {
    uint32_t mask = outputs >= 32 ? 0xffffffffu : ((1u << outputs) - 1u);
    uint32_t unknown_bits[32];
    unsigned k = 0, i;
    uint32_t first, differ = 0, combo;

    for (i = 0; i < input_count; i++) {
        if (!((known >> i) & 1u)) {
            unknown_bits[k++] = 1u << i;
        }
    }
    inputs &= known;
    if (k > ARITH_MAX_UNKNOWN) {
        *out_known = 0;
        return 0;
    }
    first = fn(bound, inputs) & mask;
    for (combo = 1; combo < (1u << k); combo++) {
        uint32_t in = inputs;

        for (i = 0; i < k; i++) {
            if ((combo >> i) & 1u) {
                in |= unknown_bits[i];
            }
        }
        differ |= (fn(bound, in) & mask) ^ first;
    }
    *out_known = mask & ~differ;
    return first & *out_known;
}

LogicValue arith_bit(uint32_t value, uint32_t known, unsigned i) {
    if (!((known >> i) & 1u)) {
        return LOGIC_X;
    }
    return (value >> i) & 1u ? LOGIC_1 : LOGIC_0;
}

int arith_bind_names(KindContext *ctx, const PartPin *pins, size_t count,
                     const char *const *names, size_t n, size_t *out) {
    size_t i;

    for (i = 0; i < n; i++) {
        if (seq_bind(ctx, pins, count, names[i], &out[i]) != 0) {
            return -1;
        }
    }
    return 0;
}

int arith_load_style(KindContext *ctx, const cJSON *json, ArithStyle *style) {
    style->open_drain = NULL;
    style->open_drain_count = 0;
    if (seq_output_style(ctx, json, &style->style) != 0) {
        return -1;
    }
    if (cJSON_GetObjectItemCaseSensitive(json, "openDrain") != NULL &&
        seq_names(ctx, json, "openDrain", 1, ARITH_MAX_OUTPUTS, &style->open_drain,
                  &style->open_drain_count) != 0) {
        return -1;
    }
    return 0;
}

int arith_bind_output(KindContext *ctx, const PartPin *pins, size_t count, const ArithStyle *style,
                      const char *name, ArithOutputs *out) {
    size_t i, pin;
    SeqOutputStyle s = style->style;

    if (name == NULL) {
        return 0;
    }
    if (out->count >= ARITH_MAX_OUTPUTS) {
        return kind_error(ctx, "more than %d outputs", ARITH_MAX_OUTPUTS);
    }
    if (seq_bind(ctx, pins, count, name, &pin) != 0) {
        return -1;
    }
    for (i = 0; i < style->open_drain_count; i++) {
        if (strcmp(style->open_drain[i], name) == 0) {
            s = SEQ_OPEN_DRAIN;
        }
    }
    out->pins[out->count] = pin;
    out->style[out->count] = s;
    out->count++;
    return 0;
}

/* Every "openDrain" name must be one of the outputs bound. */
static int check_open_drain(KindContext *ctx, const PartPin *pins, const ArithStyle *style,
                            const ArithOutputs *outs) {
    size_t i, j;

    for (i = 0; i < style->open_drain_count; i++) {
        for (j = 0; j < outs->count; j++) {
            if (strcmp(pins[outs->pins[j]].name, style->open_drain[i]) == 0) {
                break;
            }
        }
        if (j == outs->count) {
            return kind_error(ctx, "\"openDrain\": \"%s\" is not an output of the model",
                              style->open_drain[i]);
        }
    }
    return 0;
}

void arith_init_drive(const ArithOutputs *outs, ArithDrive *drive) {
    size_t i;

    for (i = 0; i < outs->count; i++) {
        drive->o[i].pin = outs->pins[i];
        drive->o[i].scheduled = 0;
        drive->o[i].driven = LOGIC_X;
    }
}

void arith_drive(Sim *sim, SimInstance *inst, const ArithOutputs *outs, ArithDrive *drive,
                 size_t i, LogicValue v, const SeqTiming *t) {
    seq_drive(sim, inst, &drive->o[i], v, outs->style[i], t);
}

void arith_read_bits(const Sim *sim, const SimInstance *inst, const size_t *pins, size_t n,
                     unsigned first, uint32_t *value, uint32_t *known) {
    size_t i;

    for (i = 0; i < n; i++) {
        int level = seq_read(sim, inst, pins[i]);

        if (level >= 0) {
            *known |= 1u << (first + i);
            *value |= (uint32_t)level << (first + i);
        }
    }
}

/* ---- compare ------------------------------------------------------------- */

#define CMP_MAX_BITS 12
#define CMP_MAX_ROWS 24

enum { REL_GT, REL_EQ, REL_LT, REL_COUNT };
static const char *const REL_KEYS[] = {"gt", "eq", "lt", NULL};

static const char *const COMPARE_KEYS[] = {
    "a", "b", "outputs", "cascade", "cascadeTable", "enable", "latch", "signed",
    "output", "openDrain", NULL
};

typedef struct {
    uint8_t in_mask, in_value;   /* over the cascade inputs present, in gt, eq, lt order */
    uint8_t out;                 /* pin levels of the outputs present, in gt, eq, lt order */
    uint8_t rel;                 /* the relation of the words the row applies to (REL_*) */
} CmpRow;

typedef struct {
    const char *const *a, *const *b;
    size_t n;
    SeqControlName outputs[REL_COUNT];   /* pin NULL: absent */
    const char *cascade[REL_COUNT];      /* NULL: absent */
    CmpRow rows[CMP_MAX_ROWS];
    size_t row_count;
    int has_rows[REL_COUNT];
    SeqControlName enable, latch, sign;
    ArithStyle style;
} CmpDevice;

typedef struct {
    ArithOutputs outs;   /* first: the kind's open-drain query reads it */
    const CmpDevice *dev;
    size_t a[CMP_MAX_BITS], b[CMP_MAX_BITS];
    size_t cascade[REL_COUNT];
    int out_slot[REL_COUNT];   /* index into outs, -1 absent */
    int out_active_high[REL_COUNT];
    unsigned cascade_count, output_count;
    SeqControl enable, latch, sign;
} CmpBound;

typedef struct {
    ArithHeader h;
    const CmpBound *b;
    LogicValue held[CMP_MAX_BITS];
    ArithDrive drive;
} CmpState;

/* Parses one "in"/"out" string of H/L (and X when allow_x) of length n. */
static int parse_levels(KindContext *ctx, const cJSON *row, const char *key, size_t n,
                        int allow_x, uint8_t *mask, uint8_t *value) {
    const char *text;
    size_t i;

    if (kind_string(ctx, row, key, &text) != 0) {
        return -1;
    }
    if (strlen(text) != n) {
        return kind_error(ctx, "\"%s\" must have %zu levels", key, n);
    }
    *mask = *value = 0;
    for (i = 0; i < n; i++) {
        if (text[i] == 'H' || text[i] == 'L') {
            *mask |= (uint8_t)(1u << i);
            *value |= (uint8_t)((text[i] == 'H') << i);
        } else if (!(allow_x && text[i] == 'X')) {
            return kind_error(ctx, "\"%s\" levels must be H, L%s", key, allow_x ? " or X" : "");
        }
    }
    return 0;
}

static void *cmp_load(KindContext *ctx, const cJSON *json) {
    CmpDevice *dev = kind_alloc(ctx, sizeof(*dev));
    const cJSON *outputs, *cascade, *table;
    size_t n_b, i, cascade_count = 0, output_count = 0;

    if (dev == NULL ||
        seq_names(ctx, json, "a", 1, CMP_MAX_BITS, &dev->a, &dev->n) != 0 ||
        seq_names(ctx, json, "b", 1, CMP_MAX_BITS, &dev->b, &n_b) != 0) {
        return NULL;
    }
    if (n_b != dev->n) {
        kind_error(ctx, "\"a\" and \"b\" must have the same number of names");
        return NULL;
    }
    outputs = kind_member(ctx, json, "outputs", 1);
    if (outputs == NULL) {
        return NULL;
    }
    kind_push(ctx, "outputs");
    if (!cJSON_IsObject(outputs) || kind_check_keys(ctx, outputs, REL_KEYS, NULL) != 0) {
        if (!cJSON_IsObject(outputs)) {
            kind_error(ctx, "must be an object of \"gt\", \"eq\", \"lt\"");
        }
        return NULL;
    }
    for (i = 0; i < REL_COUNT; i++) {
        if (seq_control_name(ctx, outputs, REL_KEYS[i], 0, 0, &dev->outputs[i]) != 0) {
            return NULL;
        }
        output_count += dev->outputs[i].pin != NULL;
    }
    kind_pop(ctx);
    if (output_count == 0) {
        kind_error(ctx, "\"outputs\" needs at least one of \"gt\", \"eq\", \"lt\"");
        return NULL;
    }
    cascade = cJSON_GetObjectItemCaseSensitive(json, "cascade");
    if (cascade != NULL) {
        kind_push(ctx, "cascade");
        if (!cJSON_IsObject(cascade)) {
            kind_error(ctx, "must be an object of \"gt\", \"eq\", \"lt\"");
            return NULL;
        }
        if (kind_check_keys(ctx, cascade, REL_KEYS, NULL) != 0) {
            return NULL;
        }
        for (i = 0; i < REL_COUNT; i++) {
            if (seq_optional_name(ctx, cascade, REL_KEYS[i], &dev->cascade[i]) != 0) {
                return NULL;
            }
            cascade_count += dev->cascade[i] != NULL;
        }
        kind_pop(ctx);
        if (cascade_count == 0) {
            kind_error(ctx, "\"cascade\" needs at least one of \"gt\", \"eq\", \"lt\"");
            return NULL;
        }
    }
    table = cJSON_GetObjectItemCaseSensitive(json, "cascadeTable");
    if (table != NULL) {
        static const char *const ROW_KEYS[] = {"when", "in", "out", NULL};
        const cJSON *row;
        unsigned combo, rel;

        if (cascade == NULL) {
            kind_error(ctx, "\"cascadeTable\" needs \"cascade\"");
            return NULL;
        }
        if (!cJSON_IsArray(table) || cJSON_GetArraySize(table) < 1 ||
            cJSON_GetArraySize(table) > CMP_MAX_ROWS) {
            kind_error(ctx, "\"cascadeTable\" must be an array of 1 to %d rows", CMP_MAX_ROWS);
            return NULL;
        }
        cJSON_ArrayForEach(row, table) {
            CmpRow *r = &dev->rows[dev->row_count];
            uint8_t out_mask;
            const char *when = "eq";

            kind_push(ctx, "cascadeTable[%zu]", dev->row_count);
            if (kind_check_keys(ctx, row, ROW_KEYS, NULL) != 0 ||
                (cJSON_GetObjectItemCaseSensitive(row, "when") != NULL &&
                 kind_string(ctx, row, "when", &when) != 0) ||
                parse_levels(ctx, row, "in", cascade_count, 1, &r->in_mask, &r->in_value) != 0 ||
                parse_levels(ctx, row, "out", output_count, 0, &out_mask, &r->out) != 0) {
                return NULL;
            }
            for (i = 0; i < REL_COUNT && strcmp(REL_KEYS[i], when) != 0; i++) {
            }
            if (i == REL_COUNT) {
                kind_error(ctx, "\"when\" must be \"gt\", \"eq\" or \"lt\", not \"%s\"", when);
                return NULL;
            }
            r->rel = (uint8_t)i;
            dev->has_rows[i] = 1;
            kind_pop(ctx);
            dev->row_count++;
        }
        /* For each relation with rows, every combination of cascade levels must match one. */
        for (rel = 0; rel < REL_COUNT; rel++) {
            for (combo = 0; dev->has_rows[rel] && combo < (1u << cascade_count); combo++) {
                for (i = 0; i < dev->row_count; i++) {
                    if (dev->rows[i].rel == rel &&
                        (combo & dev->rows[i].in_mask) == dev->rows[i].in_value) {
                        break;
                    }
                }
                if (i == dev->row_count) {
                    kind_error(ctx, "\"cascadeTable\" rows for \"%s\" miss a cascade level combination",
                               REL_KEYS[rel]);
                    return NULL;
                }
            }
        }
    }
    if (cascade != NULL && !dev->has_rows[REL_EQ]) {
        for (i = 0; i < REL_COUNT; i++) {
            if (dev->outputs[i].pin != NULL && dev->cascade[i] == NULL) {
                kind_error(ctx, "output \"%s\" has no cascade input of its name: give \"cascadeTable\"",
                           REL_KEYS[i]);
                return NULL;
            }
        }
    }
    if (seq_control_name(ctx, json, "enable", 0, 0, &dev->enable) != 0 ||
        seq_control_name(ctx, json, "latch", 0, 0, &dev->latch) != 0 ||
        seq_control_name(ctx, json, "signed", 0, 0, &dev->sign) != 0 ||
        arith_load_style(ctx, json, &dev->style) != 0) {
        return NULL;
    }
    return dev;
}

static void *cmp_bind(KindContext *ctx, const void *raw, const PartPin *pins, size_t count) {
    const CmpDevice *dev = raw;
    CmpBound *b = kind_alloc(ctx, sizeof(*b));
    size_t i;

    if (b == NULL) {
        return NULL;
    }
    b->dev = dev;
    if (arith_bind_names(ctx, pins, count, dev->a, dev->n, b->a) != 0 ||
        arith_bind_names(ctx, pins, count, dev->b, dev->n, b->b) != 0) {
        return NULL;
    }
    for (i = 0; i < REL_COUNT; i++) {
        b->out_slot[i] = -1;
        b->cascade[i] = SEQ_NO_PIN;
        if (dev->outputs[i].pin != NULL) {
            b->out_slot[i] = (int)b->outs.count;
            b->out_active_high[i] = dev->outputs[i].active_high;
            if (arith_bind_output(ctx, pins, count, &dev->style, dev->outputs[i].pin, &b->outs) != 0) {
                return NULL;
            }
            b->output_count++;
        }
        if (dev->cascade[i] != NULL) {
            if (seq_bind(ctx, pins, count, dev->cascade[i], &b->cascade[i]) != 0) {
                return NULL;
            }
            b->cascade_count++;
        }
    }
    if (check_open_drain(ctx, pins, &dev->style, &b->outs) != 0 ||
        seq_bind_control(ctx, pins, count, &dev->enable, &b->enable) != 0 ||
        seq_bind_control(ctx, pins, count, &dev->latch, &b->latch) != 0 ||
        seq_bind_control(ctx, pins, count, &dev->sign, &b->sign) != 0) {
        return NULL;
    }
    return b;
}

static ArithHeader *cmp_create(const void *bound) {
    const CmpBound *b = bound;
    CmpState *s = calloc(1, sizeof(*s));
    size_t i;

    if (s == NULL) {
        return NULL;
    }
    s->b = b;
    for (i = 0; i < CMP_MAX_BITS; i++) {
        s->held[i] = LOGIC_0;
    }
    arith_init_drive(&b->outs, &s->drive);
    return &s->h;
}

/*
 * Inputs: bits 0..n-1 A, n..2n-1 B, then the cascade inputs present (gt, eq,
 * lt order), then the signed control's assertion. Outputs: the pin levels of
 * the outputs present, in gt, eq, lt order.
 */
static uint32_t cmp_fn(const void *bound, uint32_t in) {
    const CmpBound *b = bound;
    const CmpDevice *dev = b->dev;
    unsigned n = (unsigned)dev->n;
    uint32_t mask = (1u << n) - 1u;
    int64_t a = in & mask, bb = (in >> n) & mask;
    unsigned cascade = (in >> (2 * n)) & ((1u << b->cascade_count) - 1u);
    int is_signed = b->sign.pin != SEQ_NO_PIN && ((in >> (2 * n + b->cascade_count)) & 1u);
    int asserted[REL_COUNT] = {0, 0, 0};
    uint32_t out = 0;
    unsigned i, slot = 0, rel;

    if (is_signed) {
        int64_t sign = (int64_t)1 << (n - 1);

        a = (a ^ sign) - sign;
        bb = (bb ^ sign) - sign;
    }
    rel = a > bb ? REL_GT : a < bb ? REL_LT : REL_EQ;
    if (dev->has_rows[rel]) {
        for (i = 0; i < dev->row_count; i++) {
            if (dev->rows[i].rel == rel && (cascade & dev->rows[i].in_mask) == dev->rows[i].in_value) {
                return dev->rows[i].out;   /* already pin levels */
            }
        }
    }
    if (rel != REL_EQ) {
        asserted[rel] = 1;
    } else if (b->cascade_count > 0) {
        unsigned k = 0;

        /* each output follows its same-named cascade input (levels) */
        for (i = 0; i < REL_COUNT; i++) {
            if (b->cascade[i] != SEQ_NO_PIN) {
                if (b->out_slot[i] >= 0) {
                    out |= ((cascade >> k) & 1u) << slot;
                    slot++;
                }
                k++;
            } else if (b->out_slot[i] >= 0) {
                slot++;
            }
        }
        return out;
    } else {
        asserted[REL_EQ] = 1;
    }
    for (i = 0; i < REL_COUNT; i++) {
        if (b->out_slot[i] >= 0) {
            unsigned level = asserted[i] ? (unsigned)b->out_active_high[i]
                                         : (unsigned)!b->out_active_high[i];

            out |= level << slot;
            slot++;
        }
    }
    return out;
}

static void cmp_eval(Sim *sim, SimInstance *inst, ArithHeader *h, size_t pin) {
    CmpState *s = (CmpState *)h;
    const CmpBound *b = s->b;
    unsigned n = (unsigned)b->dev->n;
    /* The latch holds while asserted; at start it holds its power-on 0. */
    int hold = b->latch.pin == SEQ_NO_PIN ? 0
               : pin == SEQ_NO_PIN        ? 1
                                          : seq_sync(sim, inst, &b->latch);
    int enable = b->enable.pin == SEQ_NO_PIN ? 1 : seq_sync(sim, inst, &b->enable);
    uint32_t value = 0, known = 0, out, out_known;
    unsigned i, k;

    for (i = 0; i < n; i++) {
        LogicValue in = seq_value(seq_read(sim, inst, b->a[i]));

        if (hold == 0) {
            s->held[i] = in;
        } else if (hold < 0 && s->held[i] != in) {
            s->held[i] = LOGIC_X;
        }
        if (s->held[i] != LOGIC_X) {
            known |= 1u << i;
            value |= (uint32_t)(s->held[i] == LOGIC_1) << i;
        }
    }
    arith_read_bits(sim, inst, b->b, n, n, &value, &known);
    k = 2 * n;
    for (i = 0; i < REL_COUNT; i++) {
        if (b->cascade[i] != SEQ_NO_PIN) {
            arith_read_bits(sim, inst, &b->cascade[i], 1, k, &value, &known);
            k++;
        }
    }
    if (b->sign.pin != SEQ_NO_PIN) {
        int asserted = seq_sync(sim, inst, &b->sign);

        if (asserted >= 0) {
            known |= 1u << k;
            value |= (uint32_t)asserted << k;
        }
    }
    out = arith_eval_unknown(cmp_fn, b, value, known, k + (b->sign.pin != SEQ_NO_PIN),
                             (unsigned)b->outs.count, &out_known);
    for (i = 0; i < REL_COUNT; i++) {
        int slot = b->out_slot[i];
        LogicValue v;

        if (slot < 0) {
            continue;
        }
        if (enable == 0) {
            v = b->out_active_high[i] ? LOGIC_0 : LOGIC_1;
        } else if (enable < 0) {
            v = LOGIC_X;
        } else {
            v = arith_bit(out, out_known, (unsigned)slot);
        }
        arith_drive(sim, inst, &b->outs, &s->drive, (size_t)slot, v, h->timing);
    }
}

static const ArithModel ARITH_COMPARE = {
    "compare", COMPARE_KEYS, cmp_load, cmp_bind, cmp_create, cmp_eval
};

/* ---- parity -------------------------------------------------------------- */

#define PAR_MAX_INPUTS 9

static const char *const PARITY_KEYS[] = {
    "inputs", "even", "odd", "bus", "error", "errorOn", "output", "openDrain", NULL
};

typedef struct {
    const char *const *inputs;
    size_t n;
    const char *even, *odd;
    const char *bus;           /* NULL: no bus */
    SeqControlName xmit;
    int drives_even;
    SeqControlName error;
    int error_on_even;
    ArithStyle style;
} ParDevice;

typedef struct {
    ArithOutputs outs;   /* first */
    size_t inputs[PAR_MAX_INPUTS];
    size_t n;
    int even_slot, odd_slot, bus_slot, error_slot;   /* -1 absent */
    size_t bus;
    SeqControl xmit;
    int drives_even;
    int error_active_high, error_on_even;
} ParBound;

typedef struct {
    ArithHeader h;
    const ParBound *b;
    ArithDrive drive;
} ParState;

static int even_odd(KindContext *ctx, const cJSON *json, const char *key, int *even) {
    const char *text;

    if (kind_string(ctx, json, key, &text) != 0) {
        return -1;
    }
    if (strcmp(text, "even") == 0) {
        *even = 1;
    } else if (strcmp(text, "odd") == 0) {
        *even = 0;
    } else {
        return kind_error(ctx, "\"%s\" must be \"even\" or \"odd\", not \"%s\"", key, text);
    }
    return 0;
}

static void *par_load(KindContext *ctx, const cJSON *json) {
    ParDevice *dev = kind_alloc(ctx, sizeof(*dev));
    const cJSON *bus;

    if (dev == NULL || seq_names(ctx, json, "inputs", 1, PAR_MAX_INPUTS, &dev->inputs, &dev->n) != 0 ||
        seq_optional_name(ctx, json, "even", &dev->even) != 0 ||
        seq_optional_name(ctx, json, "odd", &dev->odd) != 0) {
        return NULL;
    }
    bus = cJSON_GetObjectItemCaseSensitive(json, "bus");
    if (bus != NULL) {
        static const char *const BUS_KEYS[] = {"pin", "xmit", "drives", NULL};

        kind_push(ctx, "bus");
        if (!cJSON_IsObject(bus)) {
            kind_error(ctx, "must be an object {\"pin\", \"xmit\", \"drives\"}");
            return NULL;
        }
        if (kind_check_keys(ctx, bus, BUS_KEYS, NULL) != 0 ||
            kind_string(ctx, bus, "pin", &dev->bus) != 0 ||
            seq_control_name(ctx, bus, "xmit", 1, 0, &dev->xmit) != 0 ||
            even_odd(ctx, bus, "drives", &dev->drives_even) != 0) {
            return NULL;
        }
        kind_pop(ctx);
    }
    if (seq_control_name(ctx, json, "error", 0, 0, &dev->error) != 0) {
        return NULL;
    }
    if (dev->error.pin != NULL) {
        if (dev->bus == NULL) {
            kind_error(ctx, "\"error\" needs \"bus\"");
            return NULL;
        }
        if (even_odd(ctx, json, "errorOn", &dev->error_on_even) != 0) {
            return NULL;
        }
    } else if (cJSON_GetObjectItemCaseSensitive(json, "errorOn") != NULL) {
        kind_error(ctx, "\"errorOn\" needs \"error\"");
        return NULL;
    }
    if (dev->even == NULL && dev->odd == NULL && dev->bus == NULL) {
        kind_error(ctx, "a parity device needs \"even\", \"odd\" or \"bus\"");
        return NULL;
    }
    if (arith_load_style(ctx, json, &dev->style) != 0) {
        return NULL;
    }
    return dev;
}

static int bind_slot(KindContext *ctx, const PartPin *pins, size_t count, const ArithStyle *style,
                     const char *name, ArithOutputs *outs, int *slot) {
    *slot = name == NULL ? -1 : (int)outs->count;
    return arith_bind_output(ctx, pins, count, style, name, outs);
}

static void *par_bind(KindContext *ctx, const void *raw, const PartPin *pins, size_t count) {
    const ParDevice *dev = raw;
    ParBound *b = kind_alloc(ctx, sizeof(*b));

    if (b == NULL || arith_bind_names(ctx, pins, count, dev->inputs, dev->n, b->inputs) != 0 ||
        bind_slot(ctx, pins, count, &dev->style, dev->even, &b->outs, &b->even_slot) != 0 ||
        bind_slot(ctx, pins, count, &dev->style, dev->odd, &b->outs, &b->odd_slot) != 0 ||
        bind_slot(ctx, pins, count, &dev->style, dev->bus, &b->outs, &b->bus_slot) != 0 ||
        bind_slot(ctx, pins, count, &dev->style, dev->error.pin, &b->outs, &b->error_slot) != 0 ||
        check_open_drain(ctx, pins, &dev->style, &b->outs) != 0 ||
        seq_bind_control(ctx, pins, count, &dev->xmit, &b->xmit) != 0) {
        return NULL;
    }
    b->n = dev->n;
    b->bus = b->bus_slot >= 0 ? b->outs.pins[b->bus_slot] : SEQ_NO_PIN;
    if (b->bus != SEQ_NO_PIN && pins[b->bus].role != PIN_INOUT) {
        kind_error(ctx, "\"bus\": pin \"%s\" must have role inout", dev->bus);
        return NULL;
    }
    b->drives_even = dev->drives_even;
    b->error_active_high = dev->error.active_high;
    b->error_on_even = dev->error_on_even;
    return b;
}

static ArithHeader *par_create(const void *bound) {
    const ParBound *b = bound;
    ParState *s = calloc(1, sizeof(*s));

    if (s == NULL) {
        return NULL;
    }
    s->b = b;
    arith_init_drive(&b->outs, &s->drive);
    return &s->h;
}

static void par_eval(Sim *sim, SimInstance *inst, ArithHeader *h, size_t pin) {
    ParState *s = (ParState *)h;
    const ParBound *b = s->b;
    unsigned ones = 0;
    int unknown = 0;
    LogicValue even;
    size_t i;

    (void)pin;
    for (i = 0; i < b->n; i++) {
        int level = seq_read(sim, inst, b->inputs[i]);

        if (level < 0) {
            unknown = 1;
        } else {
            ones += (unsigned)level;
        }
    }
    even = unknown ? LOGIC_X : (ones % 2 == 0 ? LOGIC_1 : LOGIC_0);
    if (b->even_slot >= 0) {
        arith_drive(sim, inst, &b->outs, &s->drive, (size_t)b->even_slot, even, h->timing);
    }
    if (b->odd_slot >= 0) {
        arith_drive(sim, inst, &b->outs, &s->drive, (size_t)b->odd_slot, seq_not(even), h->timing);
    }
    if (b->bus_slot >= 0) {
        int xmit = seq_sync(sim, inst, &b->xmit);
        LogicValue bus_v, error_v;
        LogicValue inactive = b->error_active_high ? LOGIC_0 : LOGIC_1;

        if (xmit > 0) {
            bus_v = b->drives_even ? even : seq_not(even);
            error_v = inactive;
        } else if (xmit == 0) {
            int level = seq_read(sim, inst, b->bus);

            bus_v = LOGIC_Z;
            if (unknown || level < 0) {
                error_v = LOGIC_X;
            } else {
                int total_even = (ones + (unsigned)level) % 2 == 0;

                error_v = total_even == b->error_on_even ? seq_not(inactive) : inactive;
            }
        } else {
            bus_v = error_v = LOGIC_X;
        }
        arith_drive(sim, inst, &b->outs, &s->drive, (size_t)b->bus_slot, bus_v, h->timing);
        if (b->error_slot >= 0) {
            arith_drive(sim, inst, &b->outs, &s->drive, (size_t)b->error_slot, error_v, h->timing);
        }
    }
}

static const ArithModel ARITH_PARITY = {
    "parity", PARITY_KEYS, par_load, par_bind, par_create, par_eval
};

/* ---- The kind ------------------------------------------------------------- */

static const ArithModel *const MODELS[] = {
    &ARITH_COMPARE, &ARITH_PARITY, &ARITH_ADDER, &ARITH_ALU181, &ARITH_WALLACE
};

#define MODEL_COUNT (sizeof(MODELS) / sizeof(MODELS[0]))

/* "model", "timing" and every model's keys (the library checks these first). */
static const char *const DEVICE_KEYS[] = {
    "model", "timing", "output", "openDrain",
    "a", "b", "outputs", "cascade", "cascadeTable", "enable", "latch", "signed",
    "inputs", "even", "odd", "bus", "error", "errorOn",
    "sum", "cin", "cout",
    "s", "m", "cn", "f", "cn4", "p", "g", "aeqb",
    "adders",
    NULL
};
static const char *const PART_KEYS[] = {"family", "vcc", "inputMax", NULL};

static void *arith_load_device(KindContext *ctx, const cJSON *json) {
    ArithDevice *dev = kind_alloc(ctx, sizeof(*dev));
    const char *name;
    size_t i;

    if (dev == NULL || kind_string(ctx, json, "model", &name) != 0) {
        return NULL;
    }
    for (i = 0; i < MODEL_COUNT && strcmp(MODELS[i]->name, name) != 0; i++) {
    }
    if (i == MODEL_COUNT) {
        kind_error(ctx, "unknown model \"%s\"", name);
        return NULL;
    }
    dev->model = MODELS[i];
    if (seq_check_model_keys(ctx, json, name, dev->model->keys) != 0) {
        return NULL;
    }
    dev->dev = dev->model->load(ctx, json);
    if (dev->dev == NULL || seq_timing(ctx, json, NULL, 1, &dev->timing) != 0) {
        return NULL;
    }
    return dev;
}

static void *arith_bind_pinout(KindContext *ctx, const void *raw, const PartPin *pins,
                               size_t pin_count) {
    const ArithDevice *dev = raw;

    return dev->model->bind(ctx, dev->dev, pins, pin_count);
}

static int arith_load_part(KindContext *ctx, const void *raw, const void *bound,
                           const cJSON *json, Part *out) {
    const ArithDevice *dev = raw;
    const SeqTiming *entry;

    (void)bound;
    if (seq_part(ctx, &dev->timing, json, out, &entry) != 0) {
        return -1;
    }
    kind_set_part_data(out, entry);
    return 0;
}

/* Every model's bound pinout starts with its ArithOutputs. */
static int arith_pin_open_drain(const Part *part, size_t pin) {
    const ArithOutputs *outs = part_internal(part)->bound;
    size_t i;

    for (i = 0; i < outs->count; i++) {
        if (outs->pins[i] == pin) {
            return outs->style[i] == SEQ_OPEN_DRAIN;
        }
    }
    return 0;
}

/* ---- Engine callbacks ----------------------------------------------------- */

static size_t arith_pin_count(const void *config) {
    return ((const Part *)config)->pin_count;
}

static void *arith_create(const void *config) {
    const PartInternal *internal = part_internal(config);
    const ArithDevice *dev = internal->device;
    ArithHeader *h = dev->model->create(internal->bound);

    if (h != NULL) {
        h->model = dev->model;
        h->timing = internal->data;
    }
    return h;
}

static void arith_destroy(void *state) {
    free(state);
}

static void arith_start(Sim *sim, SimInstance *instance, void *state) {
    ArithHeader *h = state;

    h->model->eval(sim, instance, h, SEQ_NO_PIN);
}

static void arith_pin_changed(Sim *sim, SimInstance *instance, void *state, size_t pin,
                              LogicValue old_value, LogicValue new_value) {
    ArithHeader *h = state;

    (void)old_value;
    (void)new_value;
    h->model->eval(sim, instance, h, pin);
}

static const SimComponentType ARITH_SIM = {
    .name = "arith",
    .pin_count = arith_pin_count,
    .create = arith_create,
    .destroy = arith_destroy,
    .start = arith_start,
    .pin_changed = arith_pin_changed,
};

const PartKindOps ARITH_KIND = {
    PART_ARITH, "arith", &ARITH_SIM, DEVICE_KEYS, PART_KEYS,
    arith_load_device, arith_bind_pinout, arith_load_part, arith_pin_open_drain
};
