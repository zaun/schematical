/* models/counter_ripple.c: part of the model in models/counter.c (split by plan-68-I/J). */
#include "counter_int.h"

static const char *const RIPPLE_KEYS[] = {"units", "sections", NULL};
static const CounterExtraKey RIPPLE_EXTRA[] = {{"stage", 1}, {NULL, 0}};

static int ripple_ctl(KindContext *ctx, const cJSON *json, const char *key, int with_value,
                      RippleCtlName *out, int *present) {
    static const char *const KEYS[] = {"pins", "active", NULL};
    static const char *const VALUE_KEYS[] = {"pins", "active", "value", NULL};
    const cJSON *item = cJSON_GetObjectItemCaseSensitive(json, key);
    uint64_t value;

    *present = 0;
    if (item == NULL) {
        return 0;
    }
    kind_push(ctx, "%s", key);
    if (kind_check_keys(ctx, item, with_value ? VALUE_KEYS : KEYS, NULL) != 0 ||
        seq_names(ctx, item, "pins", 1, 4, &out->pins, &out->count) != 0 ||
        seq_active(ctx, item, "active", &out->active_high) != 0) {
        return -1;
    }
    if (with_value) {
        if (kind_uint(ctx, item, "value", 63, &value) != 0) {
            return -1;
        }
        out->value = (unsigned)value;
    }
    kind_pop(ctx);
    *present = 1;
    return 0;
}

static void *ripple_load(KindContext *ctx, const cJSON *json, unsigned *units_out) {
    static const char *const SECTION_KEYS[] = {"clock", "structure", "stages", "q", "reset", "set",
                                               NULL};
    RippleDevice *dev = kind_alloc(ctx, sizeof(*dev));
    const cJSON *sections = kind_member(ctx, json, "sections", 1);
    const cJSON *item;
    uint64_t units;
    size_t n = 0;

    if (dev == NULL || sections == NULL || kind_uint(ctx, json, "units", 8, &units) != 0) {
        return NULL;
    }
    if (units < 1) {
        kind_error(ctx, "\"units\" must be 1 to 8");
        return NULL;
    }
    if (!cJSON_IsArray(sections) || cJSON_GetArraySize(sections) < 1 ||
        cJSON_GetArraySize(sections) > RIPPLE_MAX_SECTIONS) {
        kind_error(ctx, "\"sections\" must be an array of 1 to %d sections", RIPPLE_MAX_SECTIONS);
        return NULL;
    }
    cJSON_ArrayForEach(item, sections) {
        RippleSectionDef *sd = &dev->sections[n];
        const char *structure;
        const cJSON *q;
        const cJSON *qi;
        unsigned k = 0;

        kind_push(ctx, "sections[%zu]", n);
        if (kind_check_keys(ctx, item, SECTION_KEYS, NULL) != 0 ||
            kind_string(ctx, item, "clock", &sd->clock) != 0 ||
            kind_string(ctx, item, "structure", &structure) != 0) {
            return NULL;
        }
        if (strcmp(structure, "binary") == 0) {
            uint64_t stages;

            sd->structure = R_BINARY;
            if (kind_uint(ctx, item, "stages", RIPPLE_MAX_STAGES, &stages) != 0) {
                return NULL;
            }
            if (stages < 1) {
                kind_error(ctx, "\"stages\" must be 1 to %d", RIPPLE_MAX_STAGES);
                return NULL;
            }
            sd->stages = (unsigned)stages;
        } else {
            if (strcmp(structure, "div2") == 0) {
                sd->structure = R_DIV2;
                sd->stages = 1;
            } else if (strcmp(structure, "div5") == 0) {
                sd->structure = R_DIV5;
                sd->stages = 3;
            } else if (strcmp(structure, "div6") == 0) {
                sd->structure = R_DIV6;
                sd->stages = 3;
            } else {
                kind_error(ctx, "\"structure\" must be \"binary\", \"div2\", \"div5\" or "
                                "\"div6\", not \"%s\"", structure);
                return NULL;
            }
            if (cJSON_GetObjectItemCaseSensitive(item, "stages") != NULL) {
                kind_error(ctx, "\"stages\" is only for a binary section");
                return NULL;
            }
        }
        q = kind_member(ctx, item, "q", 1);
        if (q == NULL) {
            return NULL;
        }
        if (!cJSON_IsArray(q) || (unsigned)cJSON_GetArraySize(q) != sd->stages) {
            kind_error(ctx, "\"q\" must have %u entries (a pin name or null per stage)",
                       sd->stages);
            return NULL;
        }
        cJSON_ArrayForEach(qi, q) {
            if (cJSON_IsNull(qi)) {
                sd->q[k] = NULL;
            } else if (cJSON_IsString(qi)) {
                sd->q[k] = kind_strdup(ctx, qi->valuestring);
            } else {
                kind_error(ctx, "\"q\" entries must be pin names or null");
                return NULL;
            }
            k++;
        }
        if (ripple_ctl(ctx, item, "reset", 0, &sd->reset, &sd->has_reset) != 0 ||
            ripple_ctl(ctx, item, "set", 1, &sd->set, &sd->has_set) != 0) {
            return NULL;
        }
        if (sd->has_set && sd->set.value >= (1u << sd->stages)) {
            kind_error(ctx, "set: \"value\" does not fit %u stages", sd->stages);
            return NULL;
        }
        kind_pop(ctx);
        n++;
    }
    dev->section_count = n;
    *units_out = (unsigned)units;
    return dev;
}

static int ripple_bind_ctl(KindContext *ctx, const PartPin *pins, size_t count,
                           const RippleCtlName *n, RippleCtl *c) {
    c->count = n->count;
    c->active_high = n->active_high;
    return cnt_bind_names(ctx, pins, count, n->pins, n->count, c->pins);
}

static void *ripple_bind(KindContext *ctx, const void *raw, const PartPin *pins, size_t count) {
    const RippleDevice *dev = raw;
    RippleBound *b = kind_alloc(ctx, sizeof(*b));
    size_t i;

    if (b == NULL) {
        return NULL;
    }
    for (i = 0; i < dev->section_count; i++) {
        const RippleSectionDef *sd = &dev->sections[i];
        RippleSection *s = &b->sections[i];
        unsigned k;

        kind_push(ctx, "sections[%zu]", i);
        s->def = sd;
        if (seq_bind(ctx, pins, count, sd->clock, &s->clock) != 0 ||
            (sd->has_reset && ripple_bind_ctl(ctx, pins, count, &sd->reset, &s->reset) != 0) ||
            (sd->has_set && ripple_bind_ctl(ctx, pins, count, &sd->set, &s->set) != 0)) {
            return NULL;
        }
        for (k = 0; k < sd->stages; k++) {
            if (seq_bind(ctx, pins, count, sd->q[k], &s->q[k]) != 0) {
                return NULL;
            }
        }
        kind_pop(ctx);
    }
    b->section_count = dev->section_count;
    return b;
}

static CounterHeader *ripple_create(const void *dev, const void *bound) {
    const RippleBound *b = bound;
    RippleState *s = calloc(1, sizeof(*s));
    size_t i;

    (void)dev;
    if (s == NULL) {
        return NULL;
    }
    s->b = b;
    for (i = 0; i < b->section_count; i++) {
        s->value[i] = cnt_known_bits(0, b->sections[i].def->stages);
        cnt_init_outputs(s->q[i], b->sections[i].q, b->sections[i].def->stages);
    }
    return &s->h;
}

/* A reset/set: asserted (1) when every pin is at its active level; Z reads inactive. */
static int ripple_ctl_level(const Sim *sim, const SimInstance *inst, const RippleCtl *c) {
    int all = 1;
    size_t i;

    for (i = 0; i < c->count; i++) {
        SeqControl one = {c->pins[i], c->active_high, 0};

        all = cnt_and3(all, seq_async(sim, inst, &one));
    }
    return all;
}

/* The next state of a section on its clock's falling edge (all bits known). */
static unsigned ripple_next(int structure, unsigned v, unsigned stages) {
    switch (structure) {
        case R_DIV2:
            return v ^ 1u;
        case R_DIV5: {
            /* QB = bit 0, QC = bit 1, QD = bit 2; 0-4, then the JK structure's
               illegal states (counter-b.md § 74_390): 5 -> 2, 6 -> 2, 7 -> 0. */
            static const unsigned char NEXT[8] = {1, 2, 3, 4, 0, 2, 2, 0};

            return NEXT[v & 7u];
        }
        case R_DIV6: {
            /* QB, QC a divide-by-3 (00 -> 01 -> 10 -> 00; 11 -> 00); QD (bit 2)
               toggles when QC falls (counter-b.md § 74_92). */
            unsigned low = v & 3u;
            unsigned next_low = low == 0 ? 1u : low == 1 ? 2u : 0u;
            unsigned qd = (v >> 2) & 1u;

            if ((low & 2u) && !(next_low & 2u)) {
                qd ^= 1u;
            }
            return next_low | (qd << 2);
        }
        default:
            return (v + 1) & (unsigned)cnt_mask_of(stages);
    }
}

/* Ripple depth of a stage: binary k; div5 QC 1; div6 QD 1; else 0. */
static unsigned ripple_depth(int structure, unsigned k) {
    switch (structure) {
        case R_BINARY:
            return k;
        case R_DIV5:
            return k == 1;
        case R_DIV6:
            return k == 2;
        default:
            return 0;
    }
}

static SeqBits ripple_clocked(const RippleSectionDef *sd, SeqBits v) {
    unsigned n = 1u << sd->stages;
    SeqBits r = {0, 0};
    int first = 1;
    unsigned x;

    if ((v.known & cnt_mask_of(sd->stages)) == cnt_mask_of(sd->stages)) {
        return cnt_known_bits(ripple_next(sd->structure, (unsigned)v.value, sd->stages), sd->stages);
    }
    if (sd->stages > 4) {
        /* Binary: three-valued +1. */
        return cnt_step_binary(v, sd->stages, 1);
    }
    for (x = 0; x < n; x++) {
        if (((x ^ v.value) & v.known) != 0) {
            continue;
        }
        r = first ? cnt_known_bits(ripple_next(sd->structure, x, sd->stages), sd->stages)
                  : counter_merge(r, cnt_known_bits(ripple_next(sd->structure, x, sd->stages),
                                                sd->stages));
        first = 0;
    }
    return r;
}

static void ripple_eval(Sim *sim, SimInstance *inst, RippleState *s, int at_start) {
    const RippleBound *b = s->b;
    const CounterTiming *t = s->h.timing;
    size_t i;

    for (i = 0; i < b->section_count; i++) {
        const RippleSection *sec = &b->sections[i];
        const RippleSectionDef *sd = sec->def;
        LogicValue clock = sim_read_pin(sim, inst, sec->clock);
        int edge = !at_start && seq_falling(s->last_clock[i], clock);
        int set = sd->has_set ? ripple_ctl_level(sim, inst, &sec->set) : 0;
        int reset = sd->has_reset ? ripple_ctl_level(sim, inst, &sec->reset) : 0;
        SeqBits v = s->value[i];
        SeqBits next = edge ? ripple_clocked(sd, v) : v;
        SeqBits r = {0, 0};
        int first = 1;
        int a, c;
        unsigned k;

        s->last_clock[i] = clock;
        for (a = cnt_lo(set); a <= cnt_hi(set); a++) {
            for (c = cnt_lo(reset); c <= cnt_hi(reset); c++) {
                SeqBits o = a ? cnt_known_bits(sd->set.value, sd->stages)
                              : c ? cnt_known_bits(0, sd->stages) : next;

                r = first ? o : counter_merge(r, o);
                first = 0;
            }
        }
        s->value[i] = r;
        for (k = 0; k < sd->stages; k++) {
            counter_drive(sim, inst, &s->q[i][k], seq_bit(r, k), t,
                          t->extra[0] * ripple_depth(sd->structure, k));
        }
    }
}

static void ripple_start(Sim *sim, SimInstance *inst, CounterHeader *h) {
    RippleState *s = (RippleState *)h;
    size_t i;

    for (i = 0; i < s->b->section_count; i++) {
        s->last_clock[i] = sim_read_pin(sim, inst, s->b->sections[i].clock);
    }
    ripple_eval(sim, inst, s, 1);
}

static void ripple_changed(Sim *sim, SimInstance *inst, CounterHeader *h, size_t pin,
                           LogicValue old_value, LogicValue new_value) {
    (void)pin;
    (void)old_value;
    (void)new_value;
    ripple_eval(sim, inst, (RippleState *)h, 0);
}

const CounterModel cnt_RIPPLE_MODEL = {
    "ripple", RIPPLE_KEYS, RIPPLE_EXTRA, ripple_load, ripple_bind, ripple_create, NULL,
    ripple_start, ripple_changed, NULL
};

