#include "counter_int.h"
#include "logic_alloc.h"

/* ---- Shared pieces --------------------------------------------------------- */

SeqBits counter_merge(SeqBits a, SeqBits b) {
    SeqBits r;

    r.known = a.known & b.known & ~(a.value ^ b.value);
    r.value = a.value & r.known;
    return r;
}

SeqBits counter_read_bits(const Sim *sim, const SimInstance *inst, const size_t *pins,
                          size_t count) {
    SeqBits b = {0, 0};
    size_t i;

    for (i = 0; i < count; i++) {
        seq_set_bit(&b, (unsigned)i, seq_value(seq_read(sim, inst, pins[i])));
    }
    return b;
}

void counter_drive(Sim *sim, SimInstance *inst, SeqOutput *o, LogicValue v,
                   const CounterTiming *t, SimTime extra_ps) {
    SeqTiming shifted = *t->seq;

    shifted.rise += extra_ps;
    shifted.fall += extra_ps;
    seq_drive(sim, inst, o, v, SEQ_PUSH_PULL, &shifted);
}

uint64_t cnt_mask_of(unsigned n) {
    return n >= 64 ? ~(uint64_t)0 : (((uint64_t)1 << n) - 1);
}

SeqBits cnt_known_bits(uint64_t value, unsigned n) {
    SeqBits b;

    b.known = cnt_mask_of(n);
    b.value = value & b.known;
    return b;
}

/* 1 equal, 0 differs in a known bit, -1 unknown; over n bits. */
int cnt_bits_equal(SeqBits b, uint64_t value, unsigned n) {
    uint64_t m = cnt_mask_of(n);

    if (((b.value ^ value) & b.known & m) != 0) {
        return 0;
    }
    return (b.known & m) == m ? 1 : -1;
}

int cnt_lo(int level) {
    return level < 0 ? 0 : level;
}

int cnt_hi(int level) {
    return level < 0 ? 1 : level;
}

/* Three-valued AND / NOT. */
int cnt_and3(int a, int b) {
    if (a == 0 || b == 0) {
        return 0;
    }
    return a == 1 && b == 1 ? 1 : -1;
}

int cnt_not3(int a) {
    return a < 0 ? -1 : !a;
}

int cnt_or3(int a, int b) {
    if (a == 1 || b == 1) {
        return 1;
    }
    return a == 0 && b == 0 ? 0 : -1;
}

int cnt_bind_names(KindContext *ctx, const PartPin *pins, size_t count,
                      const char *const *names, size_t n, size_t *out) {
    size_t i;

    for (i = 0; i < n; i++) {
        if (seq_bind(ctx, pins, count, names[i], &out[i]) != 0) {
            return -1;
        }
    }
    return 0;
}

void cnt_init_outputs(SeqOutput *outputs, const size_t *pins, size_t n) {
    size_t i;

    for (i = 0; i < n; i++) {
        outputs[i].pin = pins[i];
        outputs[i].scheduled = 0;
        outputs[i].driven = LOGIC_X;
    }
}

/* An optional array of control objects, at most max. */
int cnt_control_list(KindContext *ctx, const cJSON *json, const char *key, int allow_when,
                        SeqControlName *out, size_t max, size_t *count) {
    const cJSON *list = cJSON_GetObjectItemCaseSensitive(json, key);
    const cJSON *item;
    size_t n = 0;

    *count = 0;
    if (list == NULL) {
        return 0;
    }
    if (!cJSON_IsArray(list) || cJSON_GetArraySize(list) < 1 ||
        (size_t)cJSON_GetArraySize(list) > max) {
        return kind_error(ctx, "\"%s\" must be an array of 1 to %zu controls", key, max);
    }
    cJSON_ArrayForEach(item, list) {
        kind_push(ctx, "%s[%zu]", key, n);
        if (seq_control_object(ctx, item, allow_when, &out[n]) != 0) {
            return -1;
        }
        kind_pop(ctx);
        n++;
    }
    *count = n;
    return 0;
}

/* ---- The kind ------------------------------------------------------------------ */

static const CounterModel *const MODELS[] = {
    &cnt_SYNC_MODEL, &cnt_RIPPLE_MODEL, &JOHNSON_MODEL, &SEG7_MODEL, &DIVN_MODEL, &RATE_MODEL, NULL
};

/* "model", "timing" and every model's keys (the library checks these first). */
static const char *const DEVICE_KEYS[] = {
    "model", "timing", "units", "byteBus", "sections",
    /* johnson */
    "stages", "clock", "clockGate", "reset", "outputs", "carry", "data", "presetEnable", "jam",
    "qbar",
    /* seg7 */
    "clockUp", "clockDown", "segments", "glyphs", "carryPulse", "borrowPulse", "displayEnable",
    "ungatedC", "lampTest", "rippleBlank", "toggleEnable", "latchEnable", "segmentsActive",
    "segmentsOutput", "maxCount", "latchOutputs", "blank", "rbi", "rbo", "decimalPoint",
    /* divn */
    "modes", "output",
    /* rate */
    "variant", "rate", "enable", "strobe", "cascade", "clear", "set", "out", "outBar", "terminal",
    "enableOut",
    NULL
};
static const char *const PART_KEYS[] = {"family", "vcc", "inputMax", NULL};

static void *counter_load_device(KindContext *ctx, const cJSON *json) {
    CounterDevice *dev = kind_alloc(ctx, sizeof(*dev));
    const char *extra_keys[COUNTER_MAX_EXTRA + 1] = {NULL};
    const char *name;
    unsigned units = 0;
    size_t i, k;

    if (dev == NULL || kind_string(ctx, json, "model", &name) != 0) {
        return NULL;
    }
    for (i = 0; MODELS[i] != NULL && strcmp(MODELS[i]->name, name) != 0; i++) {
    }
    if (MODELS[i] == NULL) {
        kind_error(ctx, "unknown model \"%s\"", name);
        return NULL;
    }
    dev->model = MODELS[i];
    if (seq_check_model_keys(ctx, json, name, dev->model->keys) != 0) {
        return NULL;
    }
    dev->dev = dev->model->load(ctx, json, &units);
    for (k = 0; dev->model->extra != NULL && dev->model->extra[k].key != NULL; k++) {
        extra_keys[k] = dev->model->extra[k].key;
    }
    if (dev->dev == NULL || seq_timing(ctx, json, extra_keys, units, &dev->timing) != 0) {
        return NULL;
    }
    dev->timings = kind_alloc(ctx, dev->timing.count * sizeof(*dev->timings));
    if (dev->timings == NULL) {
        return NULL;
    }
    for (i = 0; i < dev->timing.count; i++) {
        dev->timings[i].seq = &dev->timing.entries[i];
        for (k = 0; dev->model->extra != NULL && dev->model->extra[k].key != NULL; k++) {
            if (seq_timing_extra(ctx, &dev->timing.entries[i], dev->model->extra[k].key,
                                 dev->model->extra[k].required, &dev->timings[i].extra[k]) != 0) {
                return NULL;
            }
        }
    }
    return dev;
}

static void *counter_bind_pinout(KindContext *ctx, const void *raw, const PartPin *pins,
                                 size_t pin_count) {
    const CounterDevice *dev = raw;

    return dev->model->bind(ctx, dev->dev, pins, pin_count);
}

static int counter_load_part(KindContext *ctx, const void *raw, const void *bound,
                             const cJSON *json, Part *out) {
    const CounterDevice *dev = raw;
    const SeqTiming *entry;

    (void)bound;
    if (seq_part(ctx, &dev->timing, json, out, &entry) != 0) {
        return -1;
    }
    kind_set_part_data(out, &dev->timings[entry - dev->timing.entries]);
    return 0;
}

/* ---- Engine callbacks ------------------------------------------------------------ */

static size_t counter_pin_count(const void *config) {
    return ((const Part *)config)->pin_count;
}

static void *counter_create(const void *config) {
    const PartInternal *internal = part_internal(config);
    const CounterDevice *dev = internal->device;
    CounterHeader *h = dev->model->create(dev->dev, internal->bound);

    if (h != NULL) {
        h->model = dev->model;
        h->timing = internal->data;
    }
    return h;
}

static void counter_destroy(void *state) {
    logic_free(state);
}

static void counter_start(Sim *sim, SimInstance *instance, void *state) {
    CounterHeader *h = state;

    h->model->start(sim, instance, h);
}

static void counter_pin_changed(Sim *sim, SimInstance *instance, void *state, size_t pin,
                                LogicValue old_value, LogicValue new_value) {
    CounterHeader *h = state;

    h->model->changed(sim, instance, h, pin, old_value, new_value);
}

static void counter_timer(Sim *sim, SimInstance *instance, void *state, uint32_t id) {
    CounterHeader *h = state;

    if (h->model->timer != NULL) {
        h->model->timer(sim, instance, h, id);
    }
}

static const SimComponentType COUNTER_SIM_MODEL = {
    .name = "counter",
    .pin_count = counter_pin_count,
    .create = counter_create,
    .destroy = counter_destroy,
    .start = counter_start,
    .pin_changed = counter_pin_changed,
    .timer = counter_timer,
};

/* Only a model with an open_drain hook has open-drain outputs (seg7 of the SN74143, plan-56-F). */
static int counter_pin_open_drain(const Part *part, size_t pin) {
    const PartInternal *internal = part_internal(part);
    const CounterDevice *dev = internal->device;

    return dev->model->open_drain != NULL && dev->model->open_drain(internal->bound, pin);
}

static int counter_pin_tristate(const Part *part, size_t pin) {
    const PartInternal *internal = part_internal(part);
    const CounterDevice *dev = internal->device;

    return dev->model->tristate != NULL && dev->model->tristate(internal->bound, pin);
}

const PartKindOps COUNTER_KIND = {
    PART_COUNTER, "counter", &COUNTER_SIM_MODEL, DEVICE_KEYS, PART_KEYS,
    counter_load_device, counter_bind_pinout, counter_load_part, counter_pin_open_drain, NULL,
    counter_pin_tristate
};
