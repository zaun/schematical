/*
 * mux: multiplexers / data selectors (plan-21-B). One kind; the device's
 * "model" key picks the model:
 *
 *   select     plain data selectors: 74x151, '153, '157, '158, '251, '253,
 *              '257, '258, 2G157, 1T157/1T158, 74AC11257, CD4512B,
 *              CD40257B, FCT2257
 *   stored, regfile
 *              selectors with storage ('298, '399, '354, '356) and the
 *              '670 register file (mux_stored.c, plan-21-C)
 *
 * Every model re-evaluates all of its outputs on any pin change. Stored bits
 * (a select latch here) hold 0 at power-on. Signal rules:
 * docs/parts-format.md "Sequential kinds" and "mux".
 */

#include <stdlib.h>
#include <string.h>

#include "kind.h"
#include "model_pins.h"
#include "mux.h"
#include "seq.h"

typedef struct {
    const MuxModel *model;
    void *dev;
    SeqTimingTable timing;
} MuxDevice;

/* ---- Shared helpers -------------------------------------------------------- */

int mux_bind_names(KindContext *ctx, const PartPin *pins, size_t count,
                   const char *const *names, size_t n, size_t *out) {
    size_t i;

    for (i = 0; i < n; i++) {
        if (seq_bind(ctx, pins, count, names[i], &out[i]) != 0) {
            return -1;
        }
    }
    return 0;
}

void mux_init_outputs(SeqOutput *outputs, const size_t *pins, size_t n) {
    size_t i;

    for (i = 0; i < n; i++) {
        outputs[i].pin = pins[i];
        outputs[i].scheduled = 0;
        outputs[i].driven = LOGIC_X;
    }
}

int mux_pin_in(size_t pin, const size_t *pins, size_t n) {
    size_t i;

    for (i = 0; i < n; i++) {
        if (pins[i] == pin) {
            return 1;
        }
    }
    return 0;
}

LogicValue mux_choose(const LogicValue *data, const int *sel, size_t select_count) {
    unsigned known = 0, value = 0, code;
    unsigned codes = 1u << select_count;
    int have = 0;
    LogicValue result = LOGIC_X;
    size_t i;

    for (i = 0; i < select_count; i++) {
        if (sel[i] >= 0) {
            known |= 1u << i;
            value |= (unsigned)sel[i] << i;
        }
    }
    for (code = 0; code < codes; code++) {
        LogicValue v;

        if ((code & known) != value) {
            continue;
        }
        v = data[code] == LOGIC_Z ? LOGIC_X : data[code];
        if (!have) {
            have = 1;
            result = v;
        } else if (result != v) {
            return LOGIC_X;
        }
    }
    return result;
}

/* ---- Selector units --------------------------------------------------------- */

static int load_unit(KindContext *ctx, const cJSON *item, MuxUnitNames *u) {
    static const char *const UNIT_KEYS[] = {"data", "select", "y", "w", "strobe", "oe", NULL};

    if (kind_check_keys(ctx, item, UNIT_KEYS, NULL) != 0 ||
        seq_names(ctx, item, "select", 1, MUX_MAX_SELECT, &u->select, &u->select_count) != 0 ||
        seq_names(ctx, item, "data", 2, MUX_MAX_DATA, &u->data, &u->data_count) != 0 ||
        seq_optional_name(ctx, item, "y", &u->y) != 0 ||
        seq_optional_name(ctx, item, "w", &u->w) != 0 ||
        seq_control_name(ctx, item, "strobe", 0, 0, &u->strobe) != 0 ||
        seq_control_name(ctx, item, "oe", 0, 0, &u->oe) != 0) {
        return -1;
    }
    if (u->data_count != ((size_t)1 << u->select_count)) {
        return kind_error(ctx, "\"data\" must have %zu names (2 to the number of select pins)",
                          (size_t)1 << u->select_count);
    }
    if (u->y == NULL && u->w == NULL) {
        return kind_error(ctx, "a unit needs \"y\" or \"w\"");
    }
    return 0;
}

int mux_load_select(KindContext *ctx, const cJSON *json, MuxSelectNames *names, unsigned *units) {
    const cJSON *list = kind_member(ctx, json, "units", 1);
    const cJSON *item;
    const char *disabled = "low";
    size_t n = 0;

    if (list == NULL) {
        return -1;
    }
    if (!cJSON_IsArray(list) || cJSON_GetArraySize(list) < 1 ||
        cJSON_GetArraySize(list) > MUX_MAX_UNITS) {
        return kind_error(ctx, "\"units\" must be an array of 1 to %d units", MUX_MAX_UNITS);
    }
    cJSON_ArrayForEach(item, list) {
        kind_push(ctx, "units[%zu]", n);
        if (load_unit(ctx, item, &names->units[n]) != 0) {
            return -1;
        }
        kind_pop(ctx);
        n++;
    }
    names->unit_count = n;
    if (cJSON_GetObjectItemCaseSensitive(json, "disabled") != NULL &&
        kind_string(ctx, json, "disabled", &disabled) != 0) {
        return -1;
    }
    if (strcmp(disabled, "z") == 0) {
        names->disabled_z = 1;
    } else if (strcmp(disabled, "low") == 0) {
        names->disabled_z = 0;
    } else {
        return kind_error(ctx, "\"disabled\" must be \"low\" or \"z\", not \"%s\"", disabled);
    }
    if (seq_optional_bool(ctx, json, "invert", 0, &names->invert) != 0 ||
        seq_output_style(ctx, json, &names->style) != 0) {
        return -1;
    }
    *units = (unsigned)n;
    return 0;
}

int mux_bind_select(KindContext *ctx, const MuxSelectNames *names, const PartPin *pins,
                    size_t count, MuxSelectBound *b) {
    size_t i;

    for (i = 0; i < names->unit_count; i++) {
        const MuxUnitNames *n = &names->units[i];
        MuxUnit *u = &b->units[i];

        kind_push(ctx, "units[%zu]", i);
        if (mux_bind_names(ctx, pins, count, n->data, n->data_count, u->data) != 0 ||
            mux_bind_names(ctx, pins, count, n->select, n->select_count, u->select) != 0 ||
            seq_bind(ctx, pins, count, n->y, &u->y) != 0 ||
            seq_bind(ctx, pins, count, n->w, &u->w) != 0 ||
            seq_bind_control(ctx, pins, count, &n->strobe, &u->strobe) != 0 ||
            seq_bind_control(ctx, pins, count, &n->oe, &u->oe) != 0) {
            return -1;
        }
        u->data_count = n->data_count;
        u->select_count = n->select_count;
        kind_pop(ctx);
    }
    b->unit_count = names->unit_count;
    b->disabled_z = names->disabled_z;
    b->invert = names->invert;
    b->style = names->style;
    return 0;
}

void mux_init_select_out(const MuxSelectBound *b, MuxUnitOut *out) {
    size_t i;

    for (i = 0; i < b->unit_count; i++) {
        mux_init_outputs(&out[i].y, &b->units[i].y, 1);
        mux_init_outputs(&out[i].w, &b->units[i].w, 1);
    }
}

void mux_drive_unit(Sim *sim, SimInstance *inst, const MuxSelectBound *b, size_t i,
                    MuxUnitOut *out, LogicValue v, int shared_oe, const SeqTiming *t) {
    const MuxUnit *u = &b->units[i];
    int oe = seq_enable(sim, inst, &u->oe);

    if (shared_oe == 0) {
        oe = 0;
    } else if (shared_oe < 0 && oe != 0) {
        oe = -1;
    }
    int strobe = seq_enable(sim, inst, &u->strobe);
    LogicValue y, w;

    if (oe == 0) {
        y = w = LOGIC_Z;
    } else if (oe < 0 || strobe < 0) {
        y = w = LOGIC_X;
    } else if (strobe == 0) {
        if (b->disabled_z) {
            y = w = LOGIC_Z;
        } else {
            y = b->invert ? LOGIC_1 : LOGIC_0;
            w = seq_not(y);
        }
    } else {
        y = b->invert ? seq_not(v) : v;
        w = seq_not(y);
    }
    seq_drive(sim, inst, &out->y, y, b->style, t);
    seq_drive(sim, inst, &out->w, w, b->style, t);
}

int mux_select_open_drain(const MuxSelectBound *b, size_t pin) {
    size_t i;

    for (i = 0; b->style == SEQ_OPEN_DRAIN && i < b->unit_count; i++) {
        if ((b->units[i].y != SEQ_NO_PIN && pin == b->units[i].y) ||
            (b->units[i].w != SEQ_NO_PIN && pin == b->units[i].w)) {
            return 1;
        }
    }
    return 0;
}

/* ---- select ------------------------------------------------------------------ */

static const char *const SELECT_KEYS[] = {
    "units", "disabled", "invert", "selectLatch", "output", NULL
};

typedef struct {
    MuxSelectNames sel;
    SeqControlName select_latch;
} SelDevice;

typedef struct {
    MuxSelectBound sel;
    SeqControl select_latch;   /* asserted = hold */
} SelBound;

typedef struct {
    MuxHeader h;
    const SelBound *b;
    LogicValue held[MUX_MAX_UNITS][MUX_MAX_SELECT];
    MuxUnitOut out[MUX_MAX_UNITS];
} SelState;

static void *sel_load(KindContext *ctx, const cJSON *json, unsigned *units) {
    SelDevice *dev = kind_alloc(ctx, sizeof(*dev));

    if (dev == NULL || mux_load_select(ctx, json, &dev->sel, units) != 0 ||
        seq_control_name(ctx, json, "selectLatch", 0, 0, &dev->select_latch) != 0) {
        return NULL;
    }
    return dev;
}

static void *sel_bind(KindContext *ctx, const void *raw, const PartPin *pins, size_t count) {
    const SelDevice *dev = raw;
    SelBound *b = kind_alloc(ctx, sizeof(*b));

    if (b == NULL || mux_bind_select(ctx, &dev->sel, pins, count, &b->sel) != 0 ||
        seq_bind_control(ctx, pins, count, &dev->select_latch, &b->select_latch) != 0) {
        return NULL;
    }
    return b;
}

static MuxHeader *sel_create(const void *bound) {
    const SelBound *b = bound;
    SelState *s = calloc(1, sizeof(*s));
    size_t i, j;

    if (s == NULL) {
        return NULL;
    }
    s->b = b;
    for (i = 0; i < MUX_MAX_UNITS; i++) {
        for (j = 0; j < MUX_MAX_SELECT; j++) {
            s->held[i][j] = LOGIC_0;
        }
    }
    mux_init_select_out(&b->sel, s->out);
    return &s->h;
}

static void sel_eval(Sim *sim, SimInstance *inst, MuxHeader *h, size_t pin,
                     LogicValue old_value, LogicValue new_value) {
    SelState *s = (SelState *)h;
    const SelBound *b = s->b;
    const MuxSelectBound *sb = &b->sel;
    /* The select latch holds while asserted; at start it holds its power-on 0. */
    int hold = b->select_latch.pin == SEQ_NO_PIN ? 0
               : pin == SEQ_NO_PIN               ? 1
                                                 : seq_sync(sim, inst, &b->select_latch);
    size_t i, j;

    (void)old_value;
    (void)new_value;
    for (i = 0; i < sb->unit_count; i++) {
        const MuxUnit *u = &sb->units[i];
        LogicValue data[MUX_MAX_DATA];
        int sel[MUX_MAX_SELECT];

        for (j = 0; j < u->select_count; j++) {
            LogicValue in = seq_value(seq_read(sim, inst, u->select[j]));

            if (hold == 0) {
                s->held[i][j] = in;
            } else if (hold < 0 && s->held[i][j] != in) {
                s->held[i][j] = LOGIC_X;
            }
            sel[j] = seq_level(s->held[i][j]);
        }
        for (j = 0; j < u->data_count; j++) {
            data[j] = seq_value(seq_read(sim, inst, u->data[j]));
        }
        mux_drive_unit(sim, inst, sb, i, &s->out[i], mux_choose(data, sel, u->select_count), 1,
                       h->timing);
    }
}

static int sel_open_drain(const void *bound, size_t pin) {
    return mux_select_open_drain(&((const SelBound *)bound)->sel, pin);
}

/* ---- The kind ------------------------------------------------------------- */

static const MuxModel MUX_SELECT = {
    "select", SELECT_KEYS, sel_load, sel_bind, sel_create, sel_eval, sel_open_drain
};

static const MuxModel *const MODELS[] = {&MUX_SELECT, &MUX_STORED, &MUX_REGFILE};

#define MODEL_COUNT (sizeof(MODELS) / sizeof(MODELS[0]))

/* "model", "timing" and every model's keys (the library checks these first). */
static const char *const DEVICE_KEYS[] = {
    "model", "timing", "output",
    "units", "disabled", "invert", "selectLatch",
    "dataLatch", "dataClock", "outputClock", "oes",
    "words", "bits", "files",
    NULL
};
static const char *const PART_KEYS[] = {"family", "vcc", "inputMax", NULL};

static void *mux_load_device(KindContext *ctx, const cJSON *json) {
    MuxDevice *dev = kind_alloc(ctx, sizeof(*dev));
    const char *name;
    unsigned units = 0;
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
    dev->dev = dev->model->load(ctx, json, &units);
    if (dev->dev == NULL || seq_timing(ctx, json, NULL, units, &dev->timing) != 0) {
        return NULL;
    }
    return dev;
}

static void *mux_bind_pinout(KindContext *ctx, const void *raw, const PartPin *pins,
                             size_t pin_count) {
    const MuxDevice *dev = raw;

    return dev->model->bind(ctx, dev->dev, pins, pin_count);
}

static int mux_load_part(KindContext *ctx, const void *raw, const void *bound,
                         const cJSON *json, Part *out) {
    const MuxDevice *dev = raw;
    const SeqTiming *entry;

    (void)bound;
    if (seq_part(ctx, &dev->timing, json, out, &entry) != 0) {
        return -1;
    }
    kind_set_part_data(out, entry);
    return 0;
}

static int mux_pin_open_drain(const Part *part, size_t pin) {
    const PartInternal *internal = part_internal(part);
    const MuxDevice *dev = internal->device;

    return dev->model->open_drain(internal->bound, pin);
}

/* ---- Engine callbacks ----------------------------------------------------- */

static size_t mux_pin_count(const void *config) {
    return ((const Part *)config)->pin_count;
}

static void *mux_create(const void *config) {
    const PartInternal *internal = part_internal(config);
    const MuxDevice *dev = internal->device;
    MuxHeader *h = dev->model->create(internal->bound);

    if (h != NULL) {
        h->model = dev->model;
        h->timing = internal->data;
    }
    return h;
}

static void mux_destroy(void *state) {
    free(state);
}

static void mux_start(Sim *sim, SimInstance *instance, void *state) {
    MuxHeader *h = state;

    h->model->eval(sim, instance, h, SEQ_NO_PIN, LOGIC_X, LOGIC_X);
}

static void mux_pin_changed(Sim *sim, SimInstance *instance, void *state, size_t pin,
                            LogicValue old_value, LogicValue new_value) {
    MuxHeader *h = state;

    h->model->eval(sim, instance, h, pin, old_value, new_value);
}

static const SimComponentType MUX_SIM = {
    .name = "mux",
    .pin_count = mux_pin_count,
    .create = mux_create,
    .destroy = mux_destroy,
    .start = mux_start,
    .pin_changed = mux_pin_changed,
};

const PartKindOps MUX_KIND = {
    PART_MUX, "mux", &MUX_SIM, DEVICE_KEYS, PART_KEYS,
    mux_load_device, mux_bind_pinout, mux_load_part, mux_pin_open_drain
};
