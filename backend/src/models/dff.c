/*
 * dff: D flip-flops (ported from backend/ff-d.c, behavior unchanged).
 *
 * Each unit captures D on its clock edge (rising, or falling for
 * "clockEdge": "falling") while PRE and CLR are inactive. PRE/CLR act at
 * once; an undriven (Z) PRE/CLR reads as inactive, X as unknown. With both
 * asserted, clearWins gives Q=0/QN=1, otherwise Q=QN=1. Power-on state is
 * Q=0, QN=1. An optional active-low output enable puts Q/QN in Z (1) or X
 * (unknown). Edges follow Verilog posedge/negedge: any move into 1 (or 0)
 * from another value. Outputs are driven strong, inertially: fall delay
 * for 0, rise delay for 1 or Z, the longer for X.
 */

#include <stdlib.h>
#include <string.h>

#include "kind.h"

#define FF_NO_PIN ((size_t)-1)
#define FF_MAX_UNITS 8

static const char *const DEVICE_KEYS[] = {
    "units", "outputEnable", "presetActiveHigh", "clearActiveHigh", "clockEdge", "timing", NULL
};
static const char *const PART_KEYS[] = {"family", NULL};

typedef struct {
    const char *d, *clk, *pre, *clr, *q, *qn;   /* NULL = no pin */
} DffUnitNames;

typedef struct {
    SimTime rise_delay;
    SimTime fall_delay;
    int clr_wins;
    PartLogic logic;
} DffTiming;

typedef struct {
    DffUnitNames units[FF_MAX_UNITS];
    size_t unit_count;
    const char *oe;
    int pre_active_high;
    int clr_active_high;
    int negative_edge;
    const char **families;
    DffTiming *timings;
    size_t timing_count;
} DffDevice;

typedef struct {
    size_t d, clk, pre, clr, q, qn;
} FfUnit;

typedef struct {
    size_t unit_count;
    FfUnit units[FF_MAX_UNITS];
    size_t oe;
    int pre_active_high;
    int clr_active_high;
    int negative_edge;
} FfLayout;

/* An optional pin name: absent -> NULL. */
static int optional_name(KindContext *ctx, const cJSON *json, const char *key, const char **out) {
    *out = NULL;
    if (cJSON_GetObjectItemCaseSensitive(json, key) == NULL) {
        return 0;
    }
    return kind_string(ctx, json, key, out);
}

static int load_edge(KindContext *ctx, const cJSON *json, int *negative) {
    const char *edge;

    if (kind_string(ctx, json, "clockEdge", &edge) != 0) {
        return -1;
    }
    if (strcmp(edge, "rising") == 0) {
        *negative = 0;
    } else if (strcmp(edge, "falling") == 0) {
        *negative = 1;
    } else {
        return kind_error(ctx, "\"clockEdge\" must be \"rising\" or \"falling\", not \"%s\"", edge);
    }
    return 0;
}

static void *dff_load_device(KindContext *ctx, const cJSON *json) {
    static const char *const UNIT_KEYS[] = {"d", "clk", "pre", "clr", "q", "qn", NULL};
    static const char *const TIMING_KEYS[] = {"rise", "fall", "clearWins", "note", NULL};
    DffDevice *device = kind_alloc(ctx, sizeof(*device));
    const cJSON *units = kind_member(ctx, json, "units", 1);
    const cJSON *timing = kind_member(ctx, json, "timing", 1);
    const cJSON *item;
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
        DffUnitNames *unit = &device->units[n];

        kind_push(ctx, "units[%zu]", n);
        if (kind_check_keys(ctx, item, UNIT_KEYS, NULL) != 0 ||
            kind_string(ctx, item, "d", &unit->d) != 0 ||
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
    if (optional_name(ctx, json, "outputEnable", &device->oe) != 0 ||
        kind_bool(ctx, json, "presetActiveHigh", &device->pre_active_high) != 0 ||
        kind_bool(ctx, json, "clearActiveHigh", &device->clr_active_high) != 0 ||
        load_edge(ctx, json, &device->negative_edge) != 0) {
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
        DffTiming *entry = &device->timings[n];
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
            kind_ns(ctx, cJSON_GetObjectItemCaseSensitive(item, "fall"), "fall", &entry->fall_delay) != 0 ||
            kind_bool(ctx, item, "clearWins", &entry->clr_wins) != 0) {
            return NULL;
        }
        if (note != NULL && !cJSON_IsString(note)) {
            kind_error(ctx, "\"note\" must be a string");
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

static void *dff_bind_pinout(KindContext *ctx, const void *raw, const PartPin *pins,
                             size_t pin_count) {
    const DffDevice *device = raw;
    FfLayout *layout = kind_alloc(ctx, sizeof(*layout));
    size_t i;

    if (layout == NULL) {
        return NULL;
    }
    for (i = 0; i < device->unit_count; i++) {
        const DffUnitNames *names = &device->units[i];
        FfUnit *unit = &layout->units[i];

        kind_push(ctx, "units[%zu]", i);
        if (bind_pin(ctx, pins, pin_count, names->d, &unit->d) != 0 ||
            bind_pin(ctx, pins, pin_count, names->clk, &unit->clk) != 0 ||
            bind_pin(ctx, pins, pin_count, names->pre, &unit->pre) != 0 ||
            bind_pin(ctx, pins, pin_count, names->clr, &unit->clr) != 0 ||
            bind_pin(ctx, pins, pin_count, names->q, &unit->q) != 0 ||
            bind_pin(ctx, pins, pin_count, names->qn, &unit->qn) != 0) {
            return NULL;
        }
        kind_pop(ctx);
    }
    if (bind_pin(ctx, pins, pin_count, device->oe, &layout->oe) != 0) {
        return NULL;
    }
    layout->unit_count = device->unit_count;
    layout->pre_active_high = device->pre_active_high;
    layout->clr_active_high = device->clr_active_high;
    layout->negative_edge = device->negative_edge;
    return layout;
}

static int dff_load_part(KindContext *ctx, const void *raw, const void *bound,
                         const cJSON *json, Part *out) {
    const DffDevice *device = raw;
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
            return 0;
        }
    }
    return kind_error(ctx, "family \"%s\" has no timing entry", family);
}

/* ---- Model --------------------------------------------------------------- */

typedef struct {
    const FfLayout *layout;
    SimTime rise_delay;
    SimTime fall_delay;
    int clr_wins;
    LogicValue q[FF_MAX_UNITS];
    LogicValue qn[FF_MAX_UNITS];
    /* Values last handed to the simulator, so unchanged outputs are not rescheduled. */
    int scheduled[FF_MAX_UNITS];
    LogicValue driven_q[FF_MAX_UNITS];
    LogicValue driven_qn[FF_MAX_UNITS];
} FfState;

static size_t dff_pin_count(const void *config) {
    return ((const Part *)config)->pin_count;
}

static void *dff_create(const void *config) {
    const PartInternal *internal = part_internal(config);
    const DffTiming *timing = internal->data;
    FfState *state = malloc(sizeof(*state));
    size_t i;

    if (state == NULL) {
        return NULL;
    }
    state->layout = internal->bound;
    state->rise_delay = timing->rise_delay;
    state->fall_delay = timing->fall_delay;
    state->clr_wins = timing->clr_wins;
    for (i = 0; i < FF_MAX_UNITS; i++) {
        state->q[i] = LOGIC_X;
        state->qn[i] = LOGIC_X;
        state->scheduled[i] = 0;
    }
    return state;
}

static void dff_destroy(void *state) {
    free(state);
}

/* 1 when asserted, 0 when not, -1 when the level is unknown. Z is not asserted. */
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
    if (clr == 1 && state->clr_wins) {
        set_unit(state, i, LOGIC_0, LOGIC_1);
    } else if (pre < 0 || clr < 0) {
        set_unit(state, i, LOGIC_X, LOGIC_X);
    } else if (pre == 1 && clr == 1) {
        set_unit(state, i, LOGIC_1, LOGIC_1);
    } else if (clr == 1) {
        set_unit(state, i, LOGIC_0, LOGIC_1);
    } else if (pre == 1) {
        set_unit(state, i, LOGIC_1, LOGIC_0);
    }
}

static void capture(Sim *sim, SimInstance *instance, FfState *state, size_t i) {
    LogicValue d = sim_read_pin(sim, instance, state->layout->units[i].d);

    if (d == LOGIC_0) {
        set_unit(state, i, LOGIC_0, LOGIC_1);
    } else if (d == LOGIC_1) {
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
    } else if (value == LOGIC_1 || value == LOGIC_Z) {
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

    if (state->layout->oe != FF_NO_PIN) {
        LogicValue oe = sim_read_pin(sim, instance, state->layout->oe);

        if (oe == LOGIC_1) {
            q = LOGIC_Z;
            qn = LOGIC_Z;
        } else if (oe != LOGIC_0) {
            q = LOGIC_X;
            qn = LOGIC_X;
        }
    }
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

static void dff_start(Sim *sim, SimInstance *instance, void *raw) {
    FfState *state = raw;
    size_t i;

    for (i = 0; i < state->layout->unit_count; i++) {
        set_unit(state, i, LOGIC_0, LOGIC_1);
        apply_async(sim, instance, state, i);
        schedule_unit(sim, instance, state, i);
    }
}

static void dff_pin_changed(Sim *sim, SimInstance *instance, void *raw, size_t pin,
                            LogicValue old_value, LogicValue new_value) {
    FfState *state = raw;
    const FfLayout *layout = state->layout;
    size_t i;

    if (pin == layout->oe) {
        for (i = 0; i < layout->unit_count; i++) {
            schedule_unit(sim, instance, state, i);
        }
        return;
    }
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

static const SimComponentType DFF_MODEL = {
    "dff", dff_pin_count, dff_create, dff_destroy, dff_start, dff_pin_changed, NULL,
    NULL, NULL, NULL, NULL
};

const PartKindOps DFF_KIND = {
    PART_DFF, "dff", &DFF_MODEL, DEVICE_KEYS, PART_KEYS,
    dff_load_device, dff_bind_pinout, dff_load_part
};
