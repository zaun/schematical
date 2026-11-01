/*
 * jkff: JK flip-flops (ported from backend/ff-jk.c, behavior unchanged).
 *
 * On its clock edge (while PRE and CLR are inactive) a unit sets, resets,
 * holds or toggles by J and K ("kInverted": the K input is K-bar, 74x109).
 * An unknown J or K gives X unless every level it could have leads to the
 * same Q. PRE/CLR act at once (Z reads inactive, X unknown; both asserted ->
 * Q=QN=1). Power-on state is Q=0, QN=1. Outputs are driven strong,
 * inertially: fall delay for 0, rise delay for 1, the longer for X.
 */

#include <stdlib.h>
#include <string.h>

#include "kind.h"

#define FF_NO_PIN ((size_t)-1)
#define FF_MAX_UNITS 2

static const char *const DEVICE_KEYS[] = {
    "units", "kInverted", "presetActiveHigh", "clearActiveHigh", "clockEdge", "timing", NULL
};
static const char *const PART_KEYS[] = {"family", "vcc", "inputMax", NULL};

typedef struct {
    const char *j, *k, *clk, *pre, *clr, *q, *qn;   /* NULL = no pin */
} JkUnitNames;

typedef struct {
    SimTime rise_delay;
    SimTime fall_delay;
    PartLogic logic;
    const PartVoltage *vcc;   /* the entry's "vcc"/"inputMax"; NULL = none */
} JkTiming;

typedef struct {
    JkUnitNames units[FF_MAX_UNITS];
    size_t unit_count;
    int k_inverted;
    int pre_active_high;
    int clr_active_high;
    int negative_edge;
    const char **families;
    JkTiming *timings;
    size_t timing_count;
} JkDevice;

typedef struct {
    size_t j, k, clk, pre, clr, q, qn;
} FfUnit;

typedef struct {
    size_t unit_count;
    FfUnit units[FF_MAX_UNITS];
    int k_inverted;
    int pre_active_high;
    int clr_active_high;
    int negative_edge;
} FfLayout;

static int optional_name(KindContext *ctx, const cJSON *json, const char *key, const char **out) {
    *out = NULL;
    if (cJSON_GetObjectItemCaseSensitive(json, key) == NULL) {
        return 0;
    }
    return kind_string(ctx, json, key, out);
}

static void *jk_load_device(KindContext *ctx, const cJSON *json) {
    static const char *const UNIT_KEYS[] = {"j", "k", "clk", "pre", "clr", "q", "qn", NULL};
    static const char *const TIMING_KEYS[] = {"rise", "fall", "note", "vcc",
                                              "inputMax", NULL};
    JkDevice *device = kind_alloc(ctx, sizeof(*device));
    const cJSON *units = kind_member(ctx, json, "units", 1);
    const cJSON *timing = kind_member(ctx, json, "timing", 1);
    const cJSON *item;
    const char *edge;
    size_t n = 0;

    if (device == NULL || units == NULL || timing == NULL) {
        return NULL;
    }
    if (!cJSON_IsArray(units) || cJSON_GetArraySize(units) < 1 ||
        cJSON_GetArraySize(units) > FF_MAX_UNITS) {
        kind_error(ctx, "\"units\" must be an array of 1 to %d units", FF_MAX_UNITS);
        return NULL;
    }
    cJSON_ArrayForEach(item, units) {
        JkUnitNames *unit = &device->units[n];

        kind_push(ctx, "units[%zu]", n);
        if (kind_check_keys(ctx, item, UNIT_KEYS, NULL) != 0 ||
            kind_string(ctx, item, "j", &unit->j) != 0 ||
            kind_string(ctx, item, "k", &unit->k) != 0 ||
            kind_string(ctx, item, "clk", &unit->clk) != 0 ||
            optional_name(ctx, item, "pre", &unit->pre) != 0 ||
            optional_name(ctx, item, "clr", &unit->clr) != 0 ||
            optional_name(ctx, item, "q", &unit->q) != 0 ||
            optional_name(ctx, item, "qn", &unit->qn) != 0) {
            return NULL;
        }
        kind_pop(ctx);
        n++;
    }
    device->unit_count = n;
    if (kind_bool(ctx, json, "kInverted", &device->k_inverted) != 0 ||
        kind_bool(ctx, json, "presetActiveHigh", &device->pre_active_high) != 0 ||
        kind_bool(ctx, json, "clearActiveHigh", &device->clr_active_high) != 0 ||
        kind_string(ctx, json, "clockEdge", &edge) != 0) {
        return NULL;
    }
    if (strcmp(edge, "rising") == 0) {
        device->negative_edge = 0;
    } else if (strcmp(edge, "falling") == 0) {
        device->negative_edge = 1;
    } else {
        kind_error(ctx, "\"clockEdge\" must be \"rising\" or \"falling\", not \"%s\"", edge);
        return NULL;
    }

    if (!cJSON_IsObject(timing) || timing->child == NULL) {
        kind_error(ctx, "\"timing\" must be a non-empty object");
        return NULL;
    }
    device->timing_count = (size_t)cJSON_GetArraySize(timing);
    device->families = kind_alloc(ctx, device->timing_count * sizeof(*device->families));
    device->timings = kind_alloc(ctx, device->timing_count * sizeof(*device->timings));
    if (device->families == NULL || device->timings == NULL) {
        return NULL;
    }
    n = 0;
    cJSON_ArrayForEach(item, timing) {
        JkTiming *entry = &device->timings[n];
        const cJSON *note = cJSON_GetObjectItemCaseSensitive(item, "note");
        const cJSON *other;

        for (other = timing->child; other != item; other = other->next) {
            if (strcmp(other->string, item->string) == 0) {
                kind_error(ctx, "duplicate key \"%s\"", item->string);
                return NULL;
            }
        }
        kind_push(ctx, "timing.%s", item->string);
        device->families[n] = kind_logic_family(ctx, item->string);
        if (device->families[n] == NULL || kind_check_keys(ctx, item, TIMING_KEYS, NULL) != 0 ||
            kind_ns(ctx, cJSON_GetObjectItemCaseSensitive(item, "rise"), "rise", &entry->rise_delay) != 0 ||
            kind_ns(ctx, cJSON_GetObjectItemCaseSensitive(item, "fall"), "fall", &entry->fall_delay) != 0) {
            return NULL;
        }
        if (note != NULL && !cJSON_IsString(note)) {
            kind_error(ctx, "\"note\" must be a string");
            return NULL;
        }
        if (kind_voltage(ctx, item, NULL, &entry->vcc) != 0) {
            return NULL;
        }
        entry->logic.family = device->families[n];
        entry->logic.units = (unsigned)device->unit_count;
        kind_pop(ctx);
        n++;
    }
    return device;
}

static int bind_pin(KindContext *ctx, const PartPin *pins, size_t pin_count, const char *name,
                    size_t *out) {
    *out = FF_NO_PIN;
    return name == NULL ? 0 : kind_pin(ctx, pins, pin_count, name, out);
}

static void *jk_bind_pinout(KindContext *ctx, const void *raw, const PartPin *pins,
                            size_t pin_count) {
    const JkDevice *device = raw;
    FfLayout *layout = kind_alloc(ctx, sizeof(*layout));
    size_t i;

    if (layout == NULL) {
        return NULL;
    }
    for (i = 0; i < device->unit_count; i++) {
        const JkUnitNames *names = &device->units[i];
        FfUnit *unit = &layout->units[i];

        kind_push(ctx, "units[%zu]", i);
        if (bind_pin(ctx, pins, pin_count, names->j, &unit->j) != 0 ||
            bind_pin(ctx, pins, pin_count, names->k, &unit->k) != 0 ||
            bind_pin(ctx, pins, pin_count, names->clk, &unit->clk) != 0 ||
            bind_pin(ctx, pins, pin_count, names->pre, &unit->pre) != 0 ||
            bind_pin(ctx, pins, pin_count, names->clr, &unit->clr) != 0 ||
            bind_pin(ctx, pins, pin_count, names->q, &unit->q) != 0 ||
            bind_pin(ctx, pins, pin_count, names->qn, &unit->qn) != 0) {
            return NULL;
        }
        kind_pop(ctx);
    }
    layout->unit_count = device->unit_count;
    layout->k_inverted = device->k_inverted;
    layout->pre_active_high = device->pre_active_high;
    layout->clr_active_high = device->clr_active_high;
    layout->negative_edge = device->negative_edge;
    return layout;
}

static int jk_load_part(KindContext *ctx, const void *raw, const void *bound,
                        const cJSON *json, Part *out) {
    const JkDevice *device = raw;
    const char *family;
    size_t i;

    (void)bound;
    if (kind_string(ctx, json, "family", &family) != 0 || kind_logic_family(ctx, family) == NULL) {
        return -1;
    }
    for (i = 0; i < device->timing_count; i++) {
        if (strcmp(device->families[i], family) == 0) {
            out->logic = &device->timings[i].logic;
            kind_set_part_data(out, &device->timings[i]);
            return kind_part_voltage(ctx, json, device->timings[i].vcc, &out->vcc);
        }
    }
    return kind_error(ctx, "family \"%s\" has no timing entry", family);
}

/* ---- Model --------------------------------------------------------------- */

typedef struct {
    const FfLayout *layout;
    SimTime rise_delay;
    SimTime fall_delay;
    LogicValue q[FF_MAX_UNITS];
    LogicValue qn[FF_MAX_UNITS];
    int scheduled[FF_MAX_UNITS];
    LogicValue driven_q[FF_MAX_UNITS];
    LogicValue driven_qn[FF_MAX_UNITS];
} FfState;

static size_t jk_pin_count(const void *config) {
    return ((const Part *)config)->pin_count;
}

static void *jk_create(const void *config) {
    const PartInternal *internal = part_internal(config);
    const JkTiming *timing = internal->data;
    FfState *state = malloc(sizeof(*state));
    size_t i;

    if (state == NULL) {
        return NULL;
    }
    state->layout = internal->bound;
    state->rise_delay = timing->rise_delay;
    state->fall_delay = timing->fall_delay;
    for (i = 0; i < FF_MAX_UNITS; i++) {
        state->q[i] = LOGIC_X;
        state->qn[i] = LOGIC_X;
        state->scheduled[i] = 0;
    }
    return state;
}

static void jk_destroy(void *state) {
    free(state);
}

static int asserted(LogicValue value, int active_high) {
    LogicValue level;

    if (value == LOGIC_Z) {
        return 0;
    }
    if (value != LOGIC_0 && value != LOGIC_1) {
        return -1;
    }
    level = active_high ? LOGIC_1 : LOGIC_0;
    return value == level;
}

static void async_status(Sim *sim, SimInstance *instance, const FfLayout *layout,
                         const FfUnit *unit, int *pre, int *clr) {
    *pre = unit->pre == FF_NO_PIN
        ? 0
        : asserted(sim_read_pin(sim, instance, unit->pre), layout->pre_active_high);
    *clr = unit->clr == FF_NO_PIN
        ? 0
        : asserted(sim_read_pin(sim, instance, unit->clr), layout->clr_active_high);
}

static void set_unit(FfState *state, size_t i, LogicValue q, LogicValue qn) {
    state->q[i] = q;
    state->qn[i] = qn;
}

static void apply_async(Sim *sim, SimInstance *instance, FfState *state, size_t i) {
    const FfUnit *unit = &state->layout->units[i];
    int pre;
    int clr;

    async_status(sim, instance, state->layout, unit, &pre, &clr);
    if (pre < 0 || clr < 0) {
        set_unit(state, i, LOGIC_X, LOGIC_X);
    } else if (pre == 1 && clr == 1) {
        set_unit(state, i, LOGIC_1, LOGIC_1);
    } else if (clr == 1) {
        set_unit(state, i, LOGIC_0, LOGIC_1);
    } else if (pre == 1) {
        set_unit(state, i, LOGIC_1, LOGIC_0);
    }
}

static LogicValue jk_next(LogicValue q, int j, int k) {
    if (j && k) {
        if (q == LOGIC_0) {
            return LOGIC_1;
        }
        if (q == LOGIC_1) {
            return LOGIC_0;
        }
        return LOGIC_X;
    }
    if (j) {
        return LOGIC_1;
    }
    if (k) {
        return LOGIC_0;
    }
    return q == LOGIC_0 || q == LOGIC_1 ? q : LOGIC_X;
}

static void capture(Sim *sim, SimInstance *instance, FfState *state, size_t i) {
    const FfLayout *layout = state->layout;
    const FfUnit *unit = &layout->units[i];
    LogicValue j = sim_read_pin(sim, instance, unit->j);
    LogicValue k = sim_read_pin(sim, instance, unit->k);
    int j_lo = j == LOGIC_1 ? 1 : 0;
    int j_hi = j == LOGIC_0 ? 0 : 1;
    int k_lo;
    int k_hi;
    LogicValue next = LOGIC_X;
    int first = 1;
    int jv;
    int kv;

    if (layout->k_inverted) {
        k_lo = k == LOGIC_0 ? 1 : 0;
        k_hi = k == LOGIC_1 ? 0 : 1;
    } else {
        k_lo = k == LOGIC_1 ? 1 : 0;
        k_hi = k == LOGIC_0 ? 0 : 1;
    }
    for (jv = j_lo; jv <= j_hi; jv++) {
        for (kv = k_lo; kv <= k_hi; kv++) {
            LogicValue value = jk_next(state->q[i], jv, kv);

            if (first) {
                next = value;
                first = 0;
            } else if (value != next) {
                next = LOGIC_X;
            }
        }
    }
    if (next == LOGIC_0) {
        set_unit(state, i, LOGIC_0, LOGIC_1);
    } else if (next == LOGIC_1) {
        set_unit(state, i, LOGIC_1, LOGIC_0);
    } else {
        set_unit(state, i, LOGIC_X, LOGIC_X);
    }
}

static void schedule_pin(Sim *sim, SimInstance *instance, const FfState *state, size_t pin,
                         LogicValue value) {
    SimTime delay;

    if (pin == FF_NO_PIN) {
        return;
    }
    if (value == LOGIC_0) {
        delay = state->fall_delay;
    } else if (value == LOGIC_1) {
        delay = state->rise_delay;
    } else {
        delay = state->rise_delay > state->fall_delay ? state->rise_delay : state->fall_delay;
    }
    sim_schedule_inertial_drive(sim, instance, pin, value, DRIVE_STRONG, delay);
}

static void schedule_unit(Sim *sim, SimInstance *instance, FfState *state, size_t i) {
    const FfUnit *unit = &state->layout->units[i];
    LogicValue q = state->q[i];
    LogicValue qn = state->qn[i];

    if (state->scheduled[i] && state->driven_q[i] == q && state->driven_qn[i] == qn) {
        return;
    }
    state->scheduled[i] = 1;
    state->driven_q[i] = q;
    state->driven_qn[i] = qn;
    schedule_pin(sim, instance, state, unit->q, q);
    schedule_pin(sim, instance, state, unit->qn, qn);
}

static int is_clock_edge(const FfLayout *layout, LogicValue old_value, LogicValue new_value) {
    if (layout->negative_edge) {
        return old_value != LOGIC_0 && new_value == LOGIC_0;
    }
    return old_value != LOGIC_1 && new_value == LOGIC_1;
}

static void jk_start(Sim *sim, SimInstance *instance, void *raw) {
    FfState *state = raw;
    size_t i;

    for (i = 0; i < state->layout->unit_count; i++) {
        set_unit(state, i, LOGIC_0, LOGIC_1);
        apply_async(sim, instance, state, i);
        schedule_unit(sim, instance, state, i);
    }
}

static void jk_pin_changed(Sim *sim, SimInstance *instance, void *raw, size_t pin,
                           LogicValue old_value, LogicValue new_value) {
    FfState *state = raw;
    const FfLayout *layout = state->layout;
    size_t i;

    for (i = 0; i < layout->unit_count; i++) {
        const FfUnit *unit = &layout->units[i];

        if (pin == unit->pre || pin == unit->clr) {
            apply_async(sim, instance, state, i);
            schedule_unit(sim, instance, state, i);
        } else if (pin == unit->clk && is_clock_edge(layout, old_value, new_value)) {
            int pre;
            int clr;

            async_status(sim, instance, layout, unit, &pre, &clr);
            if (pre == 0 && clr == 0) {
                capture(sim, instance, state, i);
                schedule_unit(sim, instance, state, i);
            }
        }
    }
}

static const SimComponentType JKFF_MODEL = {
    "jkff", jk_pin_count, jk_create, jk_destroy, jk_start, jk_pin_changed, NULL,
    NULL, NULL, NULL, NULL
};

const PartKindOps JKFF_KIND = {
    PART_JKFF, "jkff", &JKFF_MODEL, DEVICE_KEYS, PART_KEYS,
    jk_load_device, jk_bind_pinout, jk_load_part
};
