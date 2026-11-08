/*
 * arith models adder, alu181 and wallace (plan-22-C): the '283 binary full
 * adder, the '181 ALU / function generator and the '275 bit-slice Wallace
 * tree. Signal rules: docs/parts-format.md "arith".
 */

#include <stdlib.h>
#include <string.h>

#include "arith.h"
#include "kind.h"
#include "seq.h"

/* ---- adder ----------------------------------------------------------------- */

#define ADD_MAX_BITS 8

static const char *const ADDER_KEYS[] = {"a", "b", "sum", "cin", "cout", "output", NULL};

typedef struct {
    const char *const *a, *const *b, *const *sum;
    size_t n;
    const char *cin, *cout;
    ArithStyle style;
} AddDevice;

typedef struct {
    ArithOutputs outs;   /* first: sum bits, then cout */
    size_t a[ADD_MAX_BITS], b[ADD_MAX_BITS];
    size_t n;
    size_t cin;
} AddBound;

typedef struct {
    ArithHeader h;
    const AddBound *b;
    ArithDrive drive;
} AddState;

static void *add_load(KindContext *ctx, const cJSON *json) {
    AddDevice *dev = kind_alloc(ctx, sizeof(*dev));
    size_t nb, ns;

    if (dev == NULL || seq_names(ctx, json, "a", 1, ADD_MAX_BITS, &dev->a, &dev->n) != 0 ||
        seq_names(ctx, json, "b", 1, ADD_MAX_BITS, &dev->b, &nb) != 0 ||
        seq_names(ctx, json, "sum", 1, ADD_MAX_BITS, &dev->sum, &ns) != 0 ||
        seq_optional_name(ctx, json, "cin", &dev->cin) != 0 ||
        seq_optional_name(ctx, json, "cout", &dev->cout) != 0 ||
        arith_load_style(ctx, json, &dev->style) != 0) {
        return NULL;
    }
    if (nb != dev->n || ns != dev->n) {
        kind_error(ctx, "\"a\", \"b\" and \"sum\" must have the same number of names");
        return NULL;
    }
    return dev;
}

static void *add_bind(KindContext *ctx, const void *raw, const PartPin *pins, size_t count) {
    const AddDevice *dev = raw;
    AddBound *b = kind_alloc(ctx, sizeof(*b));
    size_t i;

    if (b == NULL || arith_bind_names(ctx, pins, count, dev->a, dev->n, b->a) != 0 ||
        arith_bind_names(ctx, pins, count, dev->b, dev->n, b->b) != 0 ||
        seq_bind(ctx, pins, count, dev->cin, &b->cin) != 0) {
        return NULL;
    }
    for (i = 0; i < dev->n; i++) {
        if (arith_bind_output(ctx, pins, count, &dev->style, dev->sum[i], &b->outs) != 0) {
            return NULL;
        }
    }
    if (arith_bind_output(ctx, pins, count, &dev->style, dev->cout, &b->outs) != 0) {
        return NULL;
    }
    b->n = dev->n;
    return b;
}

static ArithHeader *add_create(const void *bound) {
    const AddBound *b = bound;
    AddState *s = calloc(1, sizeof(*s));

    if (s == NULL) {
        return NULL;
    }
    s->b = b;
    arith_init_drive(&b->outs, &s->drive);
    return &s->h;
}

/* Inputs: A (n bits), B (n bits), Cin. Outputs: Σ (n bits), Cout. */
static uint32_t add_fn(const void *bound, uint32_t in) {
    const AddBound *b = bound;
    unsigned n = (unsigned)b->n;
    uint32_t mask = (1u << n) - 1u;

    return (in & mask) + ((in >> n) & mask) + ((in >> (2 * n)) & 1u);
}

static void add_eval(Sim *sim, SimInstance *inst, ArithHeader *h, size_t pin) {
    AddState *s = (AddState *)h;
    const AddBound *b = s->b;
    unsigned n = (unsigned)b->n;
    uint32_t value = 0, known = 0, out, out_known;
    size_t i;

    (void)pin;
    arith_read_bits(sim, inst, b->a, n, 0, &value, &known);
    arith_read_bits(sim, inst, b->b, n, n, &value, &known);
    if (b->cin != SEQ_NO_PIN) {
        arith_read_bits(sim, inst, &b->cin, 1, 2 * n, &value, &known);
    } else {
        known |= 1u << (2 * n);   /* no carry in: 0 */
    }
    out = arith_eval_unknown(add_fn, b, value, known, 2 * n + 1, n + 1, &out_known);
    for (i = 0; i < b->outs.count; i++) {
        arith_drive(sim, inst, &b->outs, &s->drive, i, arith_bit(out, out_known, (unsigned)i),
                    h->timing);
    }
}

const ArithModel ARITH_ADDER = {"adder", ADDER_KEYS, add_load, add_bind, add_create, add_eval};

/* ---- alu181 ------------------------------------------------------------------ */

static const char *const ALU_KEYS[] = {
    "a", "b", "s", "m", "cn", "f", "cn4", "p", "g", "aeqb", "output", "openDrain", NULL
};

typedef struct {
    const char *const *a, *const *b, *const *s, *const *f;
    const char *m, *cn, *cn4, *p, *g, *aeqb;
    ArithStyle style;
} AluDevice;

/* Output slots: F0-F3, Cn+4, P, G, A=B. */
enum { ALU_F0, ALU_CN4 = 4, ALU_P, ALU_G, ALU_AEQB, ALU_OUTPUTS };

typedef struct {
    ArithOutputs outs;   /* first, in ALU_* slot order */
    size_t a[4], b[4], s[4];
    size_t m, cn;
} AluBound;

typedef struct {
    ArithHeader h;
    const AluBound *b;
    ArithDrive drive;
} AluState;

static void *alu_load(KindContext *ctx, const cJSON *json) {
    AluDevice *dev = kind_alloc(ctx, sizeof(*dev));
    size_t n;

    if (dev == NULL || seq_names(ctx, json, "a", 4, 4, &dev->a, &n) != 0 ||
        seq_names(ctx, json, "b", 4, 4, &dev->b, &n) != 0 ||
        seq_names(ctx, json, "s", 4, 4, &dev->s, &n) != 0 ||
        seq_names(ctx, json, "f", 4, 4, &dev->f, &n) != 0 ||
        kind_string(ctx, json, "m", &dev->m) != 0 || kind_string(ctx, json, "cn", &dev->cn) != 0 ||
        kind_string(ctx, json, "cn4", &dev->cn4) != 0 || kind_string(ctx, json, "p", &dev->p) != 0 ||
        kind_string(ctx, json, "g", &dev->g) != 0 || kind_string(ctx, json, "aeqb", &dev->aeqb) != 0 ||
        arith_load_style(ctx, json, &dev->style) != 0) {
        return NULL;
    }
    return dev;
}

static void *alu_bind(KindContext *ctx, const void *raw, const PartPin *pins, size_t count) {
    const AluDevice *dev = raw;
    AluBound *b = kind_alloc(ctx, sizeof(*b));
    const char *outs[ALU_OUTPUTS];
    size_t i;

    if (b == NULL || arith_bind_names(ctx, pins, count, dev->a, 4, b->a) != 0 ||
        arith_bind_names(ctx, pins, count, dev->b, 4, b->b) != 0 ||
        arith_bind_names(ctx, pins, count, dev->s, 4, b->s) != 0 ||
        seq_bind(ctx, pins, count, dev->m, &b->m) != 0 ||
        seq_bind(ctx, pins, count, dev->cn, &b->cn) != 0) {
        return NULL;
    }
    for (i = 0; i < 4; i++) {
        outs[ALU_F0 + i] = dev->f[i];
    }
    outs[ALU_CN4] = dev->cn4;
    outs[ALU_P] = dev->p;
    outs[ALU_G] = dev->g;
    outs[ALU_AEQB] = dev->aeqb;
    for (i = 0; i < ALU_OUTPUTS; i++) {
        if (arith_bind_output(ctx, pins, count, &dev->style, outs[i], &b->outs) != 0) {
            return NULL;
        }
    }
    return b;
}

static ArithHeader *alu_create(const void *bound) {
    const AluBound *b = bound;
    AluState *s = calloc(1, sizeof(*s));

    if (s == NULL) {
        return NULL;
    }
    s->b = b;
    arith_init_drive(&b->outs, &s->drive);
    return &s->h;
}

/*
 * The '181 with active-high data (sn54ls181, Table 1). Per bit, the two
 * operands the arithmetic adds are p = A + B.S0 + NOT B.S1 and
 * g = A.NOT B.S2 + A.B.S3 (g implies p); the carry into bit 0 is NOT Cn and
 * the carries ripple as c(i+1) = g + p.c(i). Arithmetic (M low):
 * F = p XOR g XOR c; logic (M high): F = NOT (p XOR g). Cn+4 = NOT c4,
 * P = NOT (p0.p1.p2.p3), G = NOT (group generate), A=B high when every F is
 * high.
 *
 * Inputs: A0-A3 (bits 0-3), B0-B3 (4-7), S0-S3 (8-11), M (12), Cn (13).
 * Outputs: the ALU_* slots.
 */
static uint32_t alu_fn(const void *bound, uint32_t in) {
    unsigned a = in & 15u, b = (in >> 4) & 15u, s = (in >> 8) & 15u;
    unsigned m = (in >> 12) & 1u, cn = (in >> 13) & 1u;
    unsigned s0 = s & 1u, s1 = (s >> 1) & 1u, s2 = (s >> 2) & 1u, s3 = (s >> 3) & 1u;
    unsigned c = !cn, f = 0, i, gg = 0, pp = 1;
    uint32_t out;

    (void)bound;
    for (i = 0; i < 4; i++) {
        unsigned ai = (a >> i) & 1u, bi = (b >> i) & 1u, nbi = bi ^ 1u;
        unsigned p = ai | (bi & s0) | (nbi & s1);
        unsigned g = (ai & nbi & s2) | (ai & bi & s3);
        unsigned fi = m ? !(p ^ g) : (p ^ g ^ c);

        f |= fi << i;
        c = g | (p & c);
        gg = g | (p & gg);
        pp &= p;
    }
    out = f << ALU_F0;
    out |= (uint32_t)!c << ALU_CN4;
    out |= (uint32_t)!pp << ALU_P;
    out |= (uint32_t)!gg << ALU_G;
    out |= (uint32_t)(f == 15u) << ALU_AEQB;
    return out;
}

static void alu_eval(Sim *sim, SimInstance *inst, ArithHeader *h, size_t pin) {
    AluState *s = (AluState *)h;
    const AluBound *b = s->b;
    uint32_t value = 0, known = 0, out, out_known;
    size_t i;

    (void)pin;
    arith_read_bits(sim, inst, b->a, 4, 0, &value, &known);
    arith_read_bits(sim, inst, b->b, 4, 4, &value, &known);
    arith_read_bits(sim, inst, b->s, 4, 8, &value, &known);
    arith_read_bits(sim, inst, &b->m, 1, 12, &value, &known);
    arith_read_bits(sim, inst, &b->cn, 1, 13, &value, &known);
    out = arith_eval_unknown(alu_fn, b, value, known, 14, ALU_OUTPUTS, &out_known);
    for (i = 0; i < ALU_OUTPUTS; i++) {
        arith_drive(sim, inst, &b->outs, &s->drive, i, arith_bit(out, out_known, (unsigned)i),
                    h->timing);
    }
}

const ArithModel ARITH_ALU181 = {"alu181", ALU_KEYS, alu_load, alu_bind, alu_create, alu_eval};

/* ---- wallace --------------------------------------------------------------- */

#define WAL_MAX_ADDERS 8
#define WAL_MAX_NODES (WAL_MAX_ADDERS * 2)

static const char *const WALLACE_KEYS[] = {"adders", "outputs", "enable", "output", NULL};

/* An adder input: a pin (index into the pinout) or a node (an earlier adder's output). */
typedef struct {
    const char *name;
    int node;   /* node index, or -1: a pin */
} WalRef;

typedef struct {
    WalRef in[3];
    const char *sum, *carry;
} WalAdderNames;

typedef struct {
    WalAdderNames adders[WAL_MAX_ADDERS];
    size_t adder_count;
    const char *out_pins[ARITH_MAX_OUTPUTS];
    int out_nodes[ARITH_MAX_OUTPUTS];
    size_t out_count;
    SeqControlName enable;
    ArithStyle style;
} WalDevice;

typedef struct {
    int node;     /* -1: pin */
    size_t pin;
} WalInput;

typedef struct {
    ArithOutputs outs;   /* first */
    WalInput in[WAL_MAX_ADDERS][3];
    size_t adder_count;
    int out_nodes[ARITH_MAX_OUTPUTS];
    SeqControl enable;
} WalBound;

typedef struct {
    ArithHeader h;
    const WalBound *b;
    ArithDrive drive;
} WalState;

/* Node n of the adders: adder n / 2's sum (even) or carry (odd). */
static int wal_find_node(const WalDevice *dev, size_t adders, const char *name) {
    size_t i;

    for (i = 0; i < adders; i++) {
        if (strcmp(dev->adders[i].sum, name) == 0) {
            return (int)(2 * i);
        }
        if (strcmp(dev->adders[i].carry, name) == 0) {
            return (int)(2 * i + 1);
        }
    }
    return -1;
}

static void *wal_load(KindContext *ctx, const cJSON *json) {
    static const char *const ADDER_ITEM_KEYS[] = {"a", "b", "c", "sum", "carry", NULL};
    static const char *const OUT_KEYS[] = {"pin", "node", NULL};
    static const char *const ABC[] = {"a", "b", "c"};
    WalDevice *dev = kind_alloc(ctx, sizeof(*dev));
    const cJSON *list, *item;
    size_t i;

    if (dev == NULL || (list = kind_member(ctx, json, "adders", 1)) == NULL) {
        return NULL;
    }
    if (!cJSON_IsArray(list) || cJSON_GetArraySize(list) < 1 ||
        cJSON_GetArraySize(list) > WAL_MAX_ADDERS) {
        kind_error(ctx, "\"adders\" must be an array of 1 to %d full adders", WAL_MAX_ADDERS);
        return NULL;
    }
    cJSON_ArrayForEach(item, list) {
        WalAdderNames *a = &dev->adders[dev->adder_count];

        kind_push(ctx, "adders[%zu]", dev->adder_count);
        if (kind_check_keys(ctx, item, ADDER_ITEM_KEYS, NULL) != 0) {
            return NULL;
        }
        for (i = 0; i < 3; i++) {
            if (kind_string(ctx, item, ABC[i], &a->in[i].name) != 0) {
                return NULL;
            }
            /* a name an earlier adder outputs is that node; otherwise a pin */
            a->in[i].node = wal_find_node(dev, dev->adder_count, a->in[i].name);
        }
        if (kind_string(ctx, item, "sum", &a->sum) != 0 ||
            kind_string(ctx, item, "carry", &a->carry) != 0) {
            return NULL;
        }
        if (strcmp(a->sum, a->carry) == 0 || wal_find_node(dev, dev->adder_count, a->sum) >= 0 ||
            wal_find_node(dev, dev->adder_count, a->carry) >= 0) {
            kind_error(ctx, "\"sum\" and \"carry\" must be new node names");
            return NULL;
        }
        kind_pop(ctx);
        dev->adder_count++;
    }
    list = kind_member(ctx, json, "outputs", 1);
    if (list == NULL) {
        return NULL;
    }
    if (!cJSON_IsArray(list) || cJSON_GetArraySize(list) < 1 ||
        cJSON_GetArraySize(list) > ARITH_MAX_OUTPUTS) {
        kind_error(ctx, "\"outputs\" must be an array of 1 to %d {\"pin\", \"node\"}",
                   ARITH_MAX_OUTPUTS);
        return NULL;
    }
    cJSON_ArrayForEach(item, list) {
        const char *node;

        kind_push(ctx, "outputs[%zu]", dev->out_count);
        if (kind_check_keys(ctx, item, OUT_KEYS, NULL) != 0 ||
            kind_string(ctx, item, "pin", &dev->out_pins[dev->out_count]) != 0 ||
            kind_string(ctx, item, "node", &node) != 0) {
            return NULL;
        }
        dev->out_nodes[dev->out_count] = wal_find_node(dev, dev->adder_count, node);
        if (dev->out_nodes[dev->out_count] < 0) {
            kind_error(ctx, "\"node\" \"%s\" is not an adder's sum or carry", node);
            return NULL;
        }
        kind_pop(ctx);
        dev->out_count++;
    }
    if (seq_control_name(ctx, json, "enable", 0, 0, &dev->enable) != 0 ||
        arith_load_style(ctx, json, &dev->style) != 0) {
        return NULL;
    }
    return dev;
}

static void *wal_bind(KindContext *ctx, const void *raw, const PartPin *pins, size_t count) {
    const WalDevice *dev = raw;
    WalBound *b = kind_alloc(ctx, sizeof(*b));
    size_t i, j;

    if (b == NULL) {
        return NULL;
    }
    for (i = 0; i < dev->adder_count; i++) {
        for (j = 0; j < 3; j++) {
            b->in[i][j].node = dev->adders[i].in[j].node;
            b->in[i][j].pin = SEQ_NO_PIN;
            if (b->in[i][j].node < 0) {
                kind_push(ctx, "adders[%zu]", i);
                if (seq_bind(ctx, pins, count, dev->adders[i].in[j].name, &b->in[i][j].pin) != 0) {
                    return NULL;
                }
                kind_pop(ctx);
            }
        }
    }
    b->adder_count = dev->adder_count;
    for (i = 0; i < dev->out_count; i++) {
        b->out_nodes[i] = dev->out_nodes[i];
        if (arith_bind_output(ctx, pins, count, &dev->style, dev->out_pins[i], &b->outs) != 0) {
            return NULL;
        }
    }
    if (seq_bind_control(ctx, pins, count, &dev->enable, &b->enable) != 0) {
        return NULL;
    }
    return b;
}

static ArithHeader *wal_create(const void *bound) {
    const WalBound *b = bound;
    WalState *s = calloc(1, sizeof(*s));

    if (s == NULL) {
        return NULL;
    }
    s->b = b;
    arith_init_drive(&b->outs, &s->drive);
    return &s->h;
}

/* A full adder with X: the sum is X when any input is; the carry is the
   majority, known when two known inputs agree. */
static void full_adder(const LogicValue *in, LogicValue *sum, LogicValue *carry) {
    int ones = 0, zeros = 0, i;

    for (i = 0; i < 3; i++) {
        ones += in[i] == LOGIC_1;
        zeros += in[i] == LOGIC_0;
    }
    *sum = ones + zeros < 3 ? LOGIC_X : (ones % 2 ? LOGIC_1 : LOGIC_0);
    *carry = ones >= 2 ? LOGIC_1 : zeros >= 2 ? LOGIC_0 : LOGIC_X;
}

static void wal_eval(Sim *sim, SimInstance *inst, ArithHeader *h, size_t pin) {
    WalState *s = (WalState *)h;
    const WalBound *b = s->b;
    LogicValue nodes[WAL_MAX_NODES];
    int enable = b->enable.pin == SEQ_NO_PIN ? 1 : seq_enable(sim, inst, &b->enable);
    size_t i, j;

    (void)pin;
    for (i = 0; i < b->adder_count; i++) {
        LogicValue in[3];

        for (j = 0; j < 3; j++) {
            const WalInput *r = &b->in[i][j];

            in[j] = r->node >= 0 ? nodes[r->node] : seq_value(seq_read(sim, inst, r->pin));
        }
        full_adder(in, &nodes[2 * i], &nodes[2 * i + 1]);
    }
    for (i = 0; i < b->outs.count; i++) {
        arith_drive(sim, inst, &b->outs, &s->drive, i, seq_gate(nodes[b->out_nodes[i]], enable),
                    h->timing);
    }
}

const ArithModel ARITH_WALLACE = {
    "wallace", WALLACE_KEYS, wal_load, wal_bind, wal_create, wal_eval
};
