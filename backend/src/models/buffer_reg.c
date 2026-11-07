/*
 * buffer model "registered" (plan-20-E section 2): transceivers with a
 * latch or a register in each direction — '646/'648/'652/'654/'651/'653
 * (registers plus a real-time/stored select), '543/'544/16543 (latches
 * with CE/LE/OE per direction), '2952/'2953/16952 (registers with clock
 * enables).
 *
 * Per unit, direction ab (A to B) and ba (B to A) each have:
 *   store   "latch" (transparent while le, and ce if given, are asserted)
 *           or "register" (loads on the clock edge while ce is asserted)
 *   oe      the destination drives while asserted (a latch's ce gates it
 *           too, the '543 rule); or unit-level dir + oe, the '646 form
 *   select  asserted: the destination shows the stored value; deasserted:
 *           the source's real-time value. Without select: the stored value.
 *   invert  the destination shows the inverse
 * Stored bits start at 0. The store takes the source port's value whether
 * or not the part itself drives that port (the '646 stores the bus it
 * drives). Both directions driving real time at once hold the value last
 * passed, as the transceiver model does.
 */

#include <stdlib.h>
#include <string.h>

#include "buffer.h"
#include "kind.h"
#include "seq.h"

static const char *const REGISTERED_KEYS[] = {"units", "invert", "output", NULL};

typedef struct {
    int latch;                 /* 1 latch, 0 register */
    SeqControlName le, ce;     /* le: latch only; ce: optional */
    SeqClockName clk;          /* register only */
    BufEnableNames oe;
    SeqControlName select;     /* pin NULL = none */
    int invert;
} RegDirDef;

typedef struct {
    const char *const *a, *const *b;
    size_t bits;
    const char *dir;           /* NULL unless the '646 form */
    int dir_ab_high;
    BufEnableNames oe;         /* '646 form */
    RegDirDef ab, ba;
} RegUnitDef;

typedef struct {
    RegUnitDef units[BUF_MAX_UNITS];
    size_t unit_count;
} RegDevice;

typedef struct {
    SeqControl le, ce, select;
    SeqClock clk;
    BufEnable oe;
} RegDirBound;

typedef struct {
    size_t a[BUF_MAX_BITS], b[BUF_MAX_BITS];
    size_t dir;
    BufEnable oe;
    RegDirBound ab, ba;
} RegUnitBound;

typedef struct {
    RegUnitBound units[BUF_MAX_UNITS];
} RegBound;

typedef struct {
    BufHeader h;
    const RegDevice *dev;
    const RegBound *b;
    SeqOutput a[BUF_MAX_UNITS][BUF_MAX_BITS], b_out[BUF_MAX_UNITS][BUF_MAX_BITS];
    LogicValue store_ab[BUF_MAX_UNITS][BUF_MAX_BITS], store_ba[BUF_MAX_UNITS][BUF_MAX_BITS];
    LogicValue hold[BUF_MAX_UNITS][BUF_MAX_BITS];
    LogicValue clk_ab[BUF_MAX_UNITS], clk_ba[BUF_MAX_UNITS];   /* last seen clock levels */
} RegState;

/* ---- Loading -------------------------------------------------------------- */

static int load_dir(KindContext *ctx, const cJSON *unit, const char *key, int unit_form,
                    int unit_invert, RegDirDef *d) {
    static const char *const LATCH_KEYS[] = {"store", "le", "ce", "oe", "select", "invert", NULL};
    static const char *const REG_KEYS[] = {"store", "clk", "ce", "oe", "select", "invert", NULL};
    const cJSON *json = kind_member(ctx, unit, key, 1);
    const char *store;

    if (json == NULL) {
        return -1;
    }
    kind_push(ctx, "%s", key);
    if (!cJSON_IsObject(json) || kind_string(ctx, json, "store", &store) != 0) {
        return json != NULL && !cJSON_IsObject(json) ? kind_error(ctx, "must be an object") : -1;
    }
    if (strcmp(store, "latch") == 0) {
        d->latch = 1;
    } else if (strcmp(store, "register") == 0) {
        d->latch = 0;
    } else {
        return kind_error(ctx, "\"store\" must be \"latch\" or \"register\", not \"%s\"", store);
    }
    if (kind_check_keys(ctx, json, d->latch ? LATCH_KEYS : REG_KEYS, NULL) != 0 ||
        seq_control_name(ctx, json, "ce", 0, 0, &d->ce) != 0 ||
        seq_control_name(ctx, json, "select", 0, 0, &d->select) != 0 ||
        seq_optional_bool(ctx, json, "invert", unit_invert, &d->invert) != 0) {
        return -1;
    }
    if (d->latch ? seq_control_name(ctx, json, "le", 1, 0, &d->le) != 0
                 : seq_clock_name(ctx, json, "clk", 1, &d->clk) != 0) {
        return -1;
    }
    if (unit_form) {
        if (cJSON_GetObjectItemCaseSensitive(json, "oe") != NULL) {
            return kind_error(ctx, "\"oe\" is on the unit when the unit has \"dir\"");
        }
        d->oe.count = 0;
    } else if (buf_enable_names(ctx, json, "oe", 1, &d->oe) != 0) {
        return -1;
    }
    kind_pop(ctx);
    return 0;
}

static void *reg_load(KindContext *ctx, const cJSON *json, unsigned *units_out) {
    static const char *const UNIT_KEYS[] = {"a", "b", "ab", "ba", "invert", NULL};
    static const char *const DIR_UNIT_KEYS[] = {"a", "b", "dir", "oe", "ab", "ba", "invert", NULL};
    RegDevice *dev = kind_alloc(ctx, sizeof(*dev));
    const cJSON *units, *u;
    size_t i = 0, total = 0, nb;
    int invert, unit_invert;

    if (dev == NULL || (units = buf_units_array(ctx, json, &dev->unit_count)) == NULL ||
        seq_optional_bool(ctx, json, "invert", 0, &invert) != 0) {
        return NULL;
    }
    cJSON_ArrayForEach(u, units) {
        RegUnitDef *d = &dev->units[i];
        int form = cJSON_GetObjectItemCaseSensitive(u, "dir") != NULL;

        kind_push(ctx, "units[%zu]", i);
        if (kind_check_keys(ctx, u, form ? DIR_UNIT_KEYS : UNIT_KEYS, NULL) != 0 ||
            seq_names(ctx, u, "a", 1, BUF_MAX_BITS, &d->a, &d->bits) != 0 ||
            seq_names(ctx, u, "b", 1, BUF_MAX_BITS, &d->b, &nb) != 0 ||
            seq_optional_bool(ctx, u, "invert", invert, &unit_invert) != 0) {
            return NULL;
        }
        if (nb != d->bits) {
            kind_error(ctx, "\"a\" and \"b\" must have the same number of names");
            return NULL;
        }
        d->dir = NULL;
        if (form && (buf_dir_name(ctx, u, &d->dir, &d->dir_ab_high) != 0 ||
                     buf_enable_names(ctx, u, "oe", 0, &d->oe) != 0)) {
            return NULL;
        }
        if (load_dir(ctx, u, "ab", form, unit_invert, &d->ab) != 0 ||
            load_dir(ctx, u, "ba", form, unit_invert, &d->ba) != 0) {
            return NULL;
        }
        total += d->bits;
        kind_pop(ctx);
        i++;
    }
    if (total > BUF_TOTAL_BITS) {
        kind_error(ctx, "at most %d bits per part", BUF_TOTAL_BITS);
        return NULL;
    }
    *units_out = (unsigned)total;
    return dev;
}

static int bind_dir(KindContext *ctx, const PartPin *pins, size_t count, const char *key,
                    const RegDirDef *d, RegDirBound *b) {
    kind_push(ctx, "%s", key);
    if (seq_bind_control(ctx, pins, count, &d->ce, &b->ce) != 0 ||
        seq_bind_control(ctx, pins, count, &d->select, &b->select) != 0 ||
        buf_bind_enable(ctx, pins, count, &d->oe, &b->oe) != 0) {
        return -1;
    }
    if (d->latch) {
        if (seq_bind_control(ctx, pins, count, &d->le, &b->le) != 0) {
            return -1;
        }
        b->clk.pin = SEQ_NO_PIN;
    } else {
        if (seq_bind_clock(ctx, pins, count, &d->clk, &b->clk) != 0) {
            return -1;
        }
        b->le.pin = SEQ_NO_PIN;
    }
    kind_pop(ctx);
    return 0;
}

static void *reg_bind(KindContext *ctx, const void *raw, const PartPin *pins, size_t count) {
    const RegDevice *dev = raw;
    RegBound *b = kind_alloc(ctx, sizeof(*b));
    size_t i;

    if (b == NULL) {
        return NULL;
    }
    for (i = 0; i < dev->unit_count; i++) {
        const RegUnitDef *d = &dev->units[i];
        RegUnitBound *ub = &b->units[i];

        kind_push(ctx, "units[%zu]", i);
        if (buf_bind_names(ctx, pins, count, d->a, d->bits, ub->a) != 0 ||
            buf_bind_names(ctx, pins, count, d->b, d->bits, ub->b) != 0) {
            return NULL;
        }
        ub->dir = SEQ_NO_PIN;
        if (d->dir != NULL && (seq_bind(ctx, pins, count, d->dir, &ub->dir) != 0 ||
                               buf_bind_enable(ctx, pins, count, &d->oe, &ub->oe) != 0)) {
            return NULL;
        }
        if (bind_dir(ctx, pins, count, "ab", &d->ab, &ub->ab) != 0 ||
            bind_dir(ctx, pins, count, "ba", &d->ba, &ub->ba) != 0) {
            return NULL;
        }
        kind_pop(ctx);
    }
    return b;
}

static BufHeader *reg_create(const void *raw, const void *bound) {
    const RegDevice *dev = raw;
    RegState *s = calloc(1, sizeof(*s));
    size_t i, j;

    if (s == NULL) {
        return NULL;
    }
    s->dev = dev;
    s->b = bound;
    for (i = 0; i < dev->unit_count; i++) {
        buf_init_outputs(s->a[i], s->b->units[i].a, dev->units[i].bits);
        buf_init_outputs(s->b_out[i], s->b->units[i].b, dev->units[i].bits);
        for (j = 0; j < dev->units[i].bits; j++) {
            s->store_ab[i][j] = s->store_ba[i][j] = LOGIC_0;
            s->hold[i][j] = LOGIC_X;
        }
    }
    return &s->h;
}

/* ---- Running -------------------------------------------------------------- */

/* 1 asserted, 0 not, -1 unknown; a control with no pin counts as asserted. */
static int asserted(const Sim *sim, const SimInstance *inst, const SeqControl *c) {
    return c->pin == SEQ_NO_PIN ? 1 : seq_sync(sim, inst, c);
}

static int both(int x, int y) {
    if (x == 0 || y == 0) {
        return 0;
    }
    return x < 0 || y < 0 ? -1 : 1;
}

/* v where load is 1; old where 0; X where they differ and load is unknown. */
static LogicValue maybe_load(LogicValue old, LogicValue v, int load) {
    if (load == 1) {
        return v;
    }
    if (load == 0) {
        return old;
    }
    return old == v ? old : LOGIC_X;
}

/*
 * Updates one direction's store from its source port. at_start: no clock
 * edge (the levels are only recorded).
 */
static void update_store(const Sim *sim, const SimInstance *inst, const BufHeader *h,
                         const RegDirDef *d, const RegDirBound *b, const size_t *src, size_t bits,
                         LogicValue *store, LogicValue *last_clk, int at_start) {
    size_t j;
    int load;

    if (d->latch) {
        load = both(asserted(sim, inst, &b->le), asserted(sim, inst, &b->ce));
        if (at_start) {
            load = 0;
        }
    } else {
        LogicValue now = sim_read_pin(sim, inst, b->clk.pin);
        LogicValue before = *last_clk;
        int edge = seq_clock_edge(&b->clk, before, now);
        LogicValue from = b->clk.falling ? LOGIC_1 : LOGIC_0;

        *last_clk = now;
        if (at_start || before == now) {
            return;
        }
        if (edge) {
            /* From X or Z it may not have been an edge: unknown. */
            load = before == from ? 1 : -1;
        } else if ((now == LOGIC_X || now == LOGIC_Z) && before == from) {
            load = -1;   /* may have been an edge */
        } else {
            return;
        }
        load = both(load, asserted(sim, inst, &b->ce));
    }
    if (load == 0) {
        return;
    }
    for (j = 0; j < bits; j++) {
        store[j] = maybe_load(store[j], buf_port(sim, inst, h, src[j]), load);
    }
}

/* The destination's enable: 1, 0, -1. */
static int dir_enable(const Sim *sim, const SimInstance *inst, const RegUnitDef *d,
                      const RegUnitBound *u, const RegDirDef *dd, const RegDirBound *db,
                      int ab) {
    int en;

    if (d->dir != NULL) {
        int oe = buf_enabled(sim, inst, &u->oe);
        int dir = seq_read(sim, inst, u->dir);
        int to_b = dir < 0 ? -1 : (dir == d->dir_ab_high);

        if (oe == 0) {
            return 0;
        }
        if (to_b < 0) {
            return -1;
        }
        en = (ab ? to_b : !to_b) ? oe : 0;
    } else {
        en = buf_enabled(sim, inst, &db->oe);
    }
    if (dd->latch && db->ce.pin != SEQ_NO_PIN) {
        en = both(en, seq_sync(sim, inst, &db->ce));
    }
    return en;
}

/* What the destination shows: 1 stored, 0 real time, -1 unknown. */
static int shows_stored(const Sim *sim, const SimInstance *inst, const RegDirBound *db) {
    return db->select.pin == SEQ_NO_PIN ? 1 : seq_sync(sim, inst, &db->select);
}

static LogicValue pick(int stored, LogicValue s, LogicValue rt) {
    if (stored == 1) {
        return s;
    }
    if (stored == 0) {
        return rt;
    }
    return s == rt ? s : LOGIC_X;
}

static void reg_eval(Sim *sim, SimInstance *inst, BufHeader *h, int at_start) {
    RegState *s = (RegState *)h;
    size_t i, j;

    for (i = 0; i < s->dev->unit_count; i++) {
        const RegUnitDef *d = &s->dev->units[i];
        const RegUnitBound *u = &s->b->units[i];
        int en_ab, en_ba, st_ab, st_ba;

        update_store(sim, inst, h, &d->ab, &u->ab, u->a, d->bits, s->store_ab[i], &s->clk_ab[i],
                     at_start);
        update_store(sim, inst, h, &d->ba, &u->ba, u->b, d->bits, s->store_ba[i], &s->clk_ba[i],
                     at_start);
        en_ab = dir_enable(sim, inst, d, u, &d->ab, &u->ab, 1);
        en_ba = dir_enable(sim, inst, d, u, &d->ba, &u->ba, 0);
        st_ab = shows_stored(sim, inst, &u->ab);
        st_ba = shows_stored(sim, inst, &u->ba);
        for (j = 0; j < d->bits; j++) {
            LogicValue vb, va;

            if (en_ab == 1 && en_ba == 1 && st_ab == 0 && st_ba == 0) {
                /* Real time both ways: the loop holds the last value passed. */
                vb = buf_invert(s->hold[i][j], d->ab.invert);
                va = buf_invert(s->hold[i][j], d->ba.invert);
            } else {
                LogicValue rt_ab = buf_port(sim, inst, h, u->a[j]);
                LogicValue rt_ba = buf_port(sim, inst, h, u->b[j]);

                vb = buf_invert(pick(st_ab, s->store_ab[i][j], rt_ab), d->ab.invert);
                va = buf_invert(pick(st_ba, s->store_ba[i][j], rt_ba), d->ba.invert);
                if (en_ab == 1 && en_ba != 1) {
                    s->hold[i][j] = pick(st_ab, s->store_ab[i][j], rt_ab);
                } else if (en_ba == 1 && en_ab != 1) {
                    s->hold[i][j] = pick(st_ba, s->store_ba[i][j], rt_ba);
                }
            }
            buf_drive(sim, inst, h, &s->b_out[i][j], seq_gate(vb, en_ab));
            buf_drive(sim, inst, h, &s->a[i][j], seq_gate(va, en_ba));
        }
    }
}

static int reg_is_output(const void *raw, const void *bound, size_t pin) {
    const RegDevice *dev = raw;
    const RegBound *b = bound;
    size_t i, j;

    for (i = 0; i < dev->unit_count; i++) {
        for (j = 0; j < dev->units[i].bits; j++) {
            if (b->units[i].a[j] == pin || b->units[i].b[j] == pin) {
                return 1;
            }
        }
    }
    return 0;
}

const BufModel BUF_REGISTERED_MODEL = {
    "registered", REGISTERED_KEYS, reg_load, reg_bind, reg_create, reg_eval, reg_is_output
};
