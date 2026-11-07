/*
 * buffer: buffers with output enables, bus transceivers, registered
 * transceivers and level translators (plan-20-D section 2). One kind, one
 * engine model, the device's "model" key picks the behavior:
 *
 *   tristate      Y = A (or NOT A) while every enable of the unit is
 *                 asserted, else Z ('244, '541, '125, '240, 16244 ...)
 *   transceiver   two ports A and B; DIR + OE ('245 form) or one enable
 *                 per direction ('243/'623 form) ('245, '640, 16245 ...)
 *   registered    a latch or register per direction (buffer_reg.c; '646,
 *                 '652, '543, '2952)
 *
 * Signal rules: docs/parts-format.md "buffer". Every model is
 * combinational or level-sensitive except registered's clocks, so each
 * re-evaluates on any pin change. A port the part stops driving reads X
 * until its drive has let go (buf_port), so a port never reads back the
 * part's own output across a direction change.
 */

#include <stdlib.h>
#include <string.h>

#include "buffer.h"
#include "kind.h"
#include "model_pins.h"
#include "seq.h"

#define BUF_TIMER 1

/* ---- Shared helpers ------------------------------------------------------- */

int buf_enable_names(KindContext *ctx, const cJSON *json, const char *key, int required,
                     BufEnableNames *out) {
    const cJSON *item = kind_member(ctx, json, key, required);
    const cJSON *e;

    out->count = 0;
    if (item == NULL) {
        return required ? -1 : 0;
    }
    kind_push(ctx, "%s", key);
    if (cJSON_IsObject(item)) {
        if (seq_control_object(ctx, item, 0, &out->names[0]) != 0) {
            return -1;
        }
        out->count = 1;
    } else if (cJSON_IsArray(item)) {
        size_t i = 0;

        if (cJSON_GetArraySize(item) > BUF_MAX_OE) {
            return kind_error(ctx, "at most %d enables", BUF_MAX_OE);
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

int buf_bind_enable(KindContext *ctx, const PartPin *pins, size_t count,
                    const BufEnableNames *names, BufEnable *out) {
    size_t i;

    out->count = names->count;
    for (i = 0; i < names->count; i++) {
        if (seq_bind_control(ctx, pins, count, &names->names[i], &out->controls[i]) != 0) {
            return -1;
        }
    }
    return 0;
}

int buf_bind_names(KindContext *ctx, const PartPin *pins, size_t count,
                   const char *const *names, size_t n, size_t *out) {
    size_t i;

    for (i = 0; i < n; i++) {
        if (kind_pin(ctx, pins, count, names[i], &out[i]) != 0) {
            return -1;
        }
    }
    return 0;
}

int buf_enabled(const Sim *sim, const SimInstance *inst, const BufEnable *e) {
    int result = 1;
    size_t i;

    for (i = 0; i < e->count; i++) {
        int v = seq_sync(sim, inst, &e->controls[i]);

        if (v == 0) {
            return 0;
        }
        if (v < 0) {
            result = -1;
        }
    }
    return result;
}

LogicValue buf_port(const Sim *sim, const SimInstance *inst, const BufHeader *h, size_t pin) {
    LogicValue v;

    if (pin < h->pin_count && sim_now(sim) < h->own_until[pin]) {
        return LOGIC_X;
    }
    v = sim_read_pin(sim, inst, pin);
    return v == LOGIC_0 || v == LOGIC_1 ? v : LOGIC_X;
}

void buf_drive(Sim *sim, SimInstance *inst, BufHeader *h, SeqOutput *o, LogicValue v) {
    LogicValue was = o->driven;
    LogicValue now = h->style == SEQ_OPEN_DRAIN && v == LOGIC_1 ? LOGIC_Z : v;

    if (o->pin == SEQ_NO_PIN) {
        return;
    }
    seq_drive(sim, inst, o, v, h->style, h->timing);
    if (was != LOGIC_Z && now == LOGIC_Z && o->pin < h->pin_count) {
        SimTime until = sim_now(sim) + h->timing->rise + 1;

        if (h->own_until[o->pin] < until) {
            h->own_until[o->pin] = until;
        }
        if (!h->timer_pending || h->timer_at < until) {
            sim_schedule_timer(sim, inst, BUF_TIMER, until - sim_now(sim));
            h->timer_pending = 1;
            h->timer_at = until;
        }
    }
}

LogicValue buf_invert(LogicValue v, int invert) {
    return invert ? seq_not(v) : v;
}

/* Every output starts undriven (Z) and not scheduled. */
void buf_init_outputs(SeqOutput *outputs, const size_t *pins, size_t n) {
    size_t i;

    for (i = 0; i < n; i++) {
        outputs[i].pin = pins[i];
        outputs[i].scheduled = 0;
        outputs[i].driven = LOGIC_Z;
    }
}

/* "invert" on a unit, else the device's. */
static int unit_invert(KindContext *ctx, const cJSON *unit, int device_invert, int *out) {
    return seq_optional_bool(ctx, unit, "invert", device_invert, out);
}

/* ---- tristate ------------------------------------------------------------- */

static const char *const TRISTATE_KEYS[] = {"units", "invert", "output", NULL};

typedef struct {
    const char *const *a, *const *y;
    size_t bits;
    BufEnableNames oe;
    int invert;
} TriUnitDef;

typedef struct {
    TriUnitDef units[BUF_MAX_UNITS];
    size_t unit_count;
} TriDevice;

typedef struct {
    size_t a[BUF_MAX_BITS], y[BUF_MAX_BITS];
    BufEnable oe;
} TriUnitBound;

typedef struct {
    TriUnitBound units[BUF_MAX_UNITS];
} TriBound;

typedef struct {
    BufHeader h;
    const TriDevice *dev;
    const TriBound *b;
    SeqOutput y[BUF_MAX_UNITS][BUF_MAX_BITS];
} TriState;

/* "units": 1..BUF_MAX_UNITS objects; each_unit loads the model's own keys. */
const cJSON *buf_units_array(KindContext *ctx, const cJSON *json, size_t *n) {
    const cJSON *units = kind_member(ctx, json, "units", 1);

    if (units == NULL) {
        return NULL;
    }
    if (!cJSON_IsArray(units) || cJSON_GetArraySize(units) < 1 ||
        cJSON_GetArraySize(units) > BUF_MAX_UNITS) {
        kind_error(ctx, "\"units\" must be an array of 1 to %d units", BUF_MAX_UNITS);
        return NULL;
    }
    *n = (size_t)cJSON_GetArraySize(units);
    return units;
}

static void *tri_load(KindContext *ctx, const cJSON *json, unsigned *units_out) {
    static const char *const UNIT_KEYS[] = {"a", "y", "oe", "invert", NULL};
    TriDevice *dev = kind_alloc(ctx, sizeof(*dev));
    const cJSON *units, *u;
    size_t i = 0, total = 0, ny;
    int invert;

    if (dev == NULL || (units = buf_units_array(ctx, json, &dev->unit_count)) == NULL ||
        seq_optional_bool(ctx, json, "invert", 0, &invert) != 0) {
        return NULL;
    }
    cJSON_ArrayForEach(u, units) {
        TriUnitDef *d = &dev->units[i];

        kind_push(ctx, "units[%zu]", i);
        if (kind_check_keys(ctx, u, UNIT_KEYS, NULL) != 0 ||
            seq_names(ctx, u, "a", 1, BUF_MAX_BITS, &d->a, &d->bits) != 0 ||
            seq_names(ctx, u, "y", 1, BUF_MAX_BITS, &d->y, &ny) != 0 ||
            buf_enable_names(ctx, u, "oe", 0, &d->oe) != 0 ||
            unit_invert(ctx, u, invert, &d->invert) != 0) {
            return NULL;
        }
        if (ny != d->bits) {
            kind_error(ctx, "\"a\" and \"y\" must have the same number of names");
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

static void *tri_bind(KindContext *ctx, const void *raw, const PartPin *pins, size_t count) {
    const TriDevice *dev = raw;
    TriBound *b = kind_alloc(ctx, sizeof(*b));
    size_t i;

    if (b == NULL) {
        return NULL;
    }
    for (i = 0; i < dev->unit_count; i++) {
        const TriUnitDef *d = &dev->units[i];

        kind_push(ctx, "units[%zu]", i);
        if (buf_bind_names(ctx, pins, count, d->a, d->bits, b->units[i].a) != 0 ||
            buf_bind_names(ctx, pins, count, d->y, d->bits, b->units[i].y) != 0 ||
            buf_bind_enable(ctx, pins, count, &d->oe, &b->units[i].oe) != 0) {
            return NULL;
        }
        kind_pop(ctx);
    }
    return b;
}

static BufHeader *tri_create(const void *raw, const void *bound) {
    const TriDevice *dev = raw;
    TriState *s = calloc(1, sizeof(*s));
    size_t i;

    if (s == NULL) {
        return NULL;
    }
    s->dev = dev;
    s->b = bound;
    for (i = 0; i < dev->unit_count; i++) {
        buf_init_outputs(s->y[i], s->b->units[i].y, dev->units[i].bits);
    }
    return &s->h;
}

static void tri_eval(Sim *sim, SimInstance *inst, BufHeader *h, int at_start) {
    TriState *s = (TriState *)h;
    size_t i, j;

    (void)at_start;
    for (i = 0; i < s->dev->unit_count; i++) {
        const TriUnitDef *d = &s->dev->units[i];
        const TriUnitBound *u = &s->b->units[i];
        int en = buf_enabled(sim, inst, &u->oe);

        for (j = 0; j < d->bits; j++) {
            LogicValue a = buf_port(sim, inst, h, u->a[j]);

            buf_drive(sim, inst, h, &s->y[i][j], seq_gate(buf_invert(a, d->invert), en));
        }
    }
}

static int tri_is_output(const void *raw, const void *bound, size_t pin) {
    const TriDevice *dev = raw;
    const TriBound *b = bound;
    size_t i, j;

    for (i = 0; i < dev->unit_count; i++) {
        for (j = 0; j < dev->units[i].bits; j++) {
            if (b->units[i].y[j] == pin) {
                return 1;
            }
        }
    }
    return 0;
}

/* ---- transceiver ---------------------------------------------------------- */

static const char *const TRANSCEIVER_KEYS[] = {"units", "invert", "output", NULL};

typedef struct {
    const char *const *a, *const *b;
    size_t bits;
    int separate;            /* 1: oeab + oeba; 0: dir + oe */
    const char *dir;
    int dir_ab_high;         /* DIR level that means A to B */
    BufEnableNames oe, oeab, oeba;
    int invert;
} XcvrUnitDef;

typedef struct {
    XcvrUnitDef units[BUF_MAX_UNITS];
    size_t unit_count;
} XcvrDevice;

typedef struct {
    size_t a[BUF_MAX_BITS], b[BUF_MAX_BITS];
    size_t dir;
    BufEnable oe, oeab, oeba;
} XcvrUnitBound;

typedef struct {
    XcvrUnitBound units[BUF_MAX_UNITS];
} XcvrBound;

typedef struct {
    BufHeader h;
    const XcvrDevice *dev;
    const XcvrBound *b;
    SeqOutput a[BUF_MAX_UNITS][BUF_MAX_BITS], b_out[BUF_MAX_UNITS][BUF_MAX_BITS];
    LogicValue hold[BUF_MAX_UNITS][BUF_MAX_BITS];   /* last value passed, either way */
} XcvrState;

/* "dir": {"pin": name, "ab": "high"|"low"}. */
int buf_dir_name(KindContext *ctx, const cJSON *unit, const char **pin, int *ab_high) {
    static const char *const KEYS[] = {"pin", "ab", NULL};
    const cJSON *dir = kind_member(ctx, unit, "dir", 1);

    if (dir == NULL) {
        return -1;
    }
    kind_push(ctx, "dir");
    if (!cJSON_IsObject(dir)) {
        return kind_error(ctx, "must be {\"pin\", \"ab\"}");
    }
    if (kind_check_keys(ctx, dir, KEYS, NULL) != 0 || kind_string(ctx, dir, "pin", pin) != 0 ||
        seq_active(ctx, dir, "ab", ab_high) != 0) {
        return -1;
    }
    kind_pop(ctx);
    return 0;
}

static int xcvr_unit(KindContext *ctx, const cJSON *u, int device_invert, XcvrUnitDef *d) {
    static const char *const DIR_KEYS[] = {"a", "b", "dir", "oe", "invert", NULL};
    static const char *const SEP_KEYS[] = {"a", "b", "oeab", "oeba", "invert", NULL};
    size_t nb;

    d->separate = cJSON_GetObjectItemCaseSensitive(u, "dir") == NULL;
    if (kind_check_keys(ctx, u, d->separate ? SEP_KEYS : DIR_KEYS, NULL) != 0 ||
        seq_names(ctx, u, "a", 1, BUF_MAX_BITS, &d->a, &d->bits) != 0 ||
        seq_names(ctx, u, "b", 1, BUF_MAX_BITS, &d->b, &nb) != 0 ||
        unit_invert(ctx, u, device_invert, &d->invert) != 0) {
        return -1;
    }
    if (nb != d->bits) {
        return kind_error(ctx, "\"a\" and \"b\" must have the same number of names");
    }
    if (d->separate) {
        if (buf_enable_names(ctx, u, "oeab", 1, &d->oeab) != 0 ||
            buf_enable_names(ctx, u, "oeba", 1, &d->oeba) != 0) {
            return -1;
        }
    } else if (buf_dir_name(ctx, u, &d->dir, &d->dir_ab_high) != 0 ||
               buf_enable_names(ctx, u, "oe", 0, &d->oe) != 0) {
        return -1;
    }
    return 0;
}

static void *xcvr_load(KindContext *ctx, const cJSON *json, unsigned *units_out) {
    XcvrDevice *dev = kind_alloc(ctx, sizeof(*dev));
    const cJSON *units, *u;
    size_t i = 0, total = 0;
    int invert;

    if (dev == NULL || (units = buf_units_array(ctx, json, &dev->unit_count)) == NULL ||
        seq_optional_bool(ctx, json, "invert", 0, &invert) != 0) {
        return NULL;
    }
    cJSON_ArrayForEach(u, units) {
        kind_push(ctx, "units[%zu]", i);
        if (xcvr_unit(ctx, u, invert, &dev->units[i]) != 0) {
            return NULL;
        }
        total += dev->units[i].bits;
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

static void *xcvr_bind(KindContext *ctx, const void *raw, const PartPin *pins, size_t count) {
    const XcvrDevice *dev = raw;
    XcvrBound *b = kind_alloc(ctx, sizeof(*b));
    size_t i;

    if (b == NULL) {
        return NULL;
    }
    for (i = 0; i < dev->unit_count; i++) {
        const XcvrUnitDef *d = &dev->units[i];
        XcvrUnitBound *ub = &b->units[i];

        kind_push(ctx, "units[%zu]", i);
        if (buf_bind_names(ctx, pins, count, d->a, d->bits, ub->a) != 0 ||
            buf_bind_names(ctx, pins, count, d->b, d->bits, ub->b) != 0) {
            return NULL;
        }
        if (d->separate) {
            ub->dir = SEQ_NO_PIN;
            if (buf_bind_enable(ctx, pins, count, &d->oeab, &ub->oeab) != 0 ||
                buf_bind_enable(ctx, pins, count, &d->oeba, &ub->oeba) != 0) {
                return NULL;
            }
        } else if (seq_bind(ctx, pins, count, d->dir, &ub->dir) != 0 ||
                   buf_bind_enable(ctx, pins, count, &d->oe, &ub->oe) != 0) {
            return NULL;
        }
        kind_pop(ctx);
    }
    return b;
}

static BufHeader *xcvr_create(const void *raw, const void *bound) {
    const XcvrDevice *dev = raw;
    XcvrState *s = calloc(1, sizeof(*s));
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
            s->hold[i][j] = LOGIC_X;
        }
    }
    return &s->h;
}

/*
 * The enables of each direction: 1 on, 0 off, -1 unknown. The '245 form
 * turns on the direction DIR names while OE is asserted; X on DIR makes
 * both directions unknown (while OE is not deasserted).
 */
static void xcvr_dirs(const Sim *sim, const SimInstance *inst, const XcvrUnitDef *d,
                      const XcvrUnitBound *u, int *ab, int *ba) {
    if (d->separate) {
        *ab = buf_enabled(sim, inst, &u->oeab);
        *ba = buf_enabled(sim, inst, &u->oeba);
    } else {
        int oe = buf_enabled(sim, inst, &u->oe);
        int dir = seq_read(sim, inst, u->dir);
        int to_b = dir < 0 ? -1 : (dir == d->dir_ab_high);

        if (oe == 0) {
            *ab = *ba = 0;
        } else if (to_b < 0) {
            *ab = *ba = -1;
        } else {
            /* DIR known: the other direction is off; this one is on or unknown. */
            *ab = to_b ? oe : 0;
            *ba = to_b ? 0 : oe;
        }
    }
}

static void xcvr_eval(Sim *sim, SimInstance *inst, BufHeader *h, int at_start) {
    XcvrState *s = (XcvrState *)h;
    size_t i, j;

    (void)at_start;
    for (i = 0; i < s->dev->unit_count; i++) {
        const XcvrUnitDef *d = &s->dev->units[i];
        const XcvrUnitBound *u = &s->b->units[i];
        int ab, ba;

        xcvr_dirs(sim, inst, d, u, &ab, &ba);
        for (j = 0; j < d->bits; j++) {
            LogicValue va = LOGIC_Z, vb = LOGIC_Z;

            if (ab == 1 && ba == 1) {
                /* Both buffers on: the loop holds the value last passed. */
                va = vb = s->hold[i][j];
            } else {
                LogicValue from_a = buf_invert(buf_port(sim, inst, h, u->a[j]), d->invert);
                LogicValue from_b = buf_invert(buf_port(sim, inst, h, u->b[j]), d->invert);

                if (ab == 1 && ba == 0) {
                    s->hold[i][j] = from_a;
                } else if (ba == 1 && ab == 0) {
                    s->hold[i][j] = from_b;
                }
                vb = ab == 1 ? from_a : ab < 0 ? LOGIC_X : LOGIC_Z;
                va = ba == 1 ? from_b : ba < 0 ? LOGIC_X : LOGIC_Z;
                if (ab == 1 && ba < 0) {
                    vb = LOGIC_X;
                }
                if (ba == 1 && ab < 0) {
                    va = LOGIC_X;
                }
            }
            buf_drive(sim, inst, h, &s->b_out[i][j], vb);
            buf_drive(sim, inst, h, &s->a[i][j], va);
        }
    }
}

static int xcvr_is_output(const void *raw, const void *bound, size_t pin) {
    const XcvrDevice *dev = raw;
    const XcvrBound *b = bound;
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

/* ---- The kind ------------------------------------------------------------- */

static const BufModel TRISTATE_MODEL = {
    "tristate", TRISTATE_KEYS, tri_load, tri_bind, tri_create, tri_eval, tri_is_output
};
static const BufModel TRANSCEIVER_MODEL = {
    "transceiver", TRANSCEIVER_KEYS, xcvr_load, xcvr_bind, xcvr_create, xcvr_eval, xcvr_is_output
};

static const BufModel *const MODELS[] = {
    &TRISTATE_MODEL, &TRANSCEIVER_MODEL, &BUF_REGISTERED_MODEL,
};

#define MODEL_COUNT (sizeof(MODELS) / sizeof(MODELS[0]))

static const char *const DEVICE_KEYS[] = {"model", "timing", "units", "invert", "output", NULL};
static const char *const PART_KEYS[] = {"family", "vcc", "inputMax", NULL};

typedef struct {
    const BufModel *model;
    void *dev;
    SeqTimingTable timing;
    SeqOutputStyle style;
} BufDevice;

static const BufModel *find_model(const char *name) {
    size_t i;

    for (i = 0; i < MODEL_COUNT; i++) {
        if (strcmp(MODELS[i]->name, name) == 0) {
            return MODELS[i];
        }
    }
    return NULL;
}

static void *buffer_load_device(KindContext *ctx, const cJSON *json) {
    BufDevice *dev = kind_alloc(ctx, sizeof(*dev));
    const char *name;
    unsigned units = 0;

    if (dev == NULL || kind_string(ctx, json, "model", &name) != 0) {
        return NULL;
    }
    dev->model = find_model(name);
    if (dev->model == NULL) {
        kind_error(ctx, "unknown model \"%s\"", name);
        return NULL;
    }
    if (seq_check_model_keys(ctx, json, name, dev->model->keys) != 0 ||
        seq_output_style(ctx, json, &dev->style) != 0) {
        return NULL;
    }
    dev->dev = dev->model->load(ctx, json, &units);
    if (dev->dev == NULL || seq_timing(ctx, json, NULL, units, &dev->timing) != 0) {
        return NULL;
    }
    return dev;
}

static void *buffer_bind_pinout(KindContext *ctx, const void *raw, const PartPin *pins,
                                size_t pin_count) {
    const BufDevice *dev = raw;

    return dev->model->bind(ctx, dev->dev, pins, pin_count);
}

static int buffer_load_part(KindContext *ctx, const void *raw, const void *bound,
                            const cJSON *json, Part *out) {
    const BufDevice *dev = raw;
    const SeqTiming *entry;

    (void)bound;
    if (seq_part(ctx, &dev->timing, json, out, &entry) != 0) {
        return -1;
    }
    kind_set_part_data(out, entry);
    return 0;
}

static int buffer_pin_open_drain(const Part *part, size_t pin) {
    const PartInternal *internal = part_internal(part);
    const BufDevice *dev = internal->device;

    return dev->style == SEQ_OPEN_DRAIN && dev->model->is_output != NULL &&
           dev->model->is_output(dev->dev, internal->bound, pin);
}

/* ---- Engine callbacks ----------------------------------------------------- */

static size_t buffer_pin_count(const void *config) {
    return ((const Part *)config)->pin_count;
}

static void *buffer_create(const void *config) {
    const Part *part = config;
    const PartInternal *internal = part_internal(part);
    const BufDevice *dev = internal->device;
    BufHeader *h = dev->model->create(dev->dev, internal->bound);

    if (h == NULL) {
        return NULL;
    }
    h->own_until = calloc(part->pin_count ? part->pin_count : 1, sizeof(*h->own_until));
    if (h->own_until == NULL) {
        free(h);
        return NULL;
    }
    h->model = dev->model;
    h->timing = internal->data;
    h->style = dev->style;
    h->pin_count = part->pin_count;
    return h;
}

static void buffer_destroy(void *state) {
    BufHeader *h = state;

    if (h != NULL) {
        free(h->own_until);
    }
    free(state);
}

static void buffer_start(Sim *sim, SimInstance *instance, void *state) {
    BufHeader *h = state;

    h->model->eval(sim, instance, h, 1);
}

static void buffer_pin_changed(Sim *sim, SimInstance *instance, void *state, size_t pin,
                               LogicValue old_value, LogicValue new_value) {
    BufHeader *h = state;

    (void)pin;
    (void)old_value;
    (void)new_value;
    h->model->eval(sim, instance, h, 0);
}

static void buffer_timer(Sim *sim, SimInstance *instance, void *state, uint32_t timer_id) {
    BufHeader *h = state;

    (void)timer_id;
    if (sim_now(sim) >= h->timer_at) {
        h->timer_pending = 0;
    }
    h->model->eval(sim, instance, h, 0);
}

static const SimComponentType BUFFER_MODEL = {
    .name = "buffer",
    .pin_count = buffer_pin_count,
    .create = buffer_create,
    .destroy = buffer_destroy,
    .start = buffer_start,
    .pin_changed = buffer_pin_changed,
    .timer = buffer_timer,
};

const PartKindOps BUFFER_KIND = {
    PART_BUFFER, "buffer", &BUFFER_MODEL, DEVICE_KEYS, PART_KEYS,
    buffer_load_device, buffer_bind_pinout, buffer_load_part, buffer_pin_open_drain
};
