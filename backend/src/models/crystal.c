/*
 * crystal (plan-44-E): quartz crystals and ceramic resonators. A two-terminal
 * frequency reference; a resonator with built-in load capacitors has a third
 * pin, the common one. In the logic simulation a crystal is a clock source on
 * its own two terminals: from time 0 it drives X1 at the level (0 first) and
 * X2 at the complement, both weakly, so a clock input wired to either
 * terminal counts it and any strong driver on a terminal overrides it. The
 * clock is exactly `hz`: the facts (load capacitance, tolerance, stability,
 * ESR) change nothing in the simulation. The half period is rounded to whole
 * picoseconds like the oscillator's.
 */

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "kind.h"
#include "logic_alloc.h"

static const char *const DEVICE_KEYS[] = {"type", "terminals", NULL};
static const char *const PART_KEYS[] = {"hz", "loadPf", "tolerancePpm", "stabilityPpm",
                                         "esrOhms", "capacitors", NULL};

static const char *const TYPES[] = {"crystal", "resonator", NULL};

typedef struct {
    const char *type;
    const char *terminals[2];
} CrystalDevice;

static void *crystal_load_device(KindContext *ctx, const cJSON *json) {
    CrystalDevice *device = kind_alloc(ctx, sizeof(*device));
    const char *const *names;
    size_t count;
    size_t i;

    if (device == NULL || kind_string(ctx, json, "type", &device->type) != 0) {
        return NULL;
    }
    for (i = 0; TYPES[i] != NULL && strcmp(TYPES[i], device->type) != 0; i++) {
    }
    if (TYPES[i] == NULL) {
        kind_error(ctx, "unknown crystal type \"%s\"", device->type);
        return NULL;
    }
    device->type = TYPES[i];
    if (kind_names(ctx, json, "terminals", &names, &count) != 0) {
        return NULL;
    }
    if (count != 2) {
        kind_error(ctx, "\"terminals\" must name 2 pins");
        return NULL;
    }
    device->terminals[0] = names[0];
    device->terminals[1] = names[1];
    return device;
}

static void *crystal_bind_pinout(KindContext *ctx, const void *raw, const PartPin *pins,
                                 size_t pin_count) {
    const CrystalDevice *device = raw;
    size_t *bound;
    size_t i;

    if (pin_count < 2 || pin_count > 4) {
        kind_error(ctx, "a crystal has 2 to 4 pins");
        return NULL;
    }
    bound = kind_alloc(ctx, 2 * sizeof(*bound));
    if (bound == NULL || kind_pin(ctx, pins, pin_count, device->terminals[0], &bound[0]) != 0 ||
        kind_pin(ctx, pins, pin_count, device->terminals[1], &bound[1]) != 0) {
        return NULL;
    }
    if (bound[0] == bound[1]) {
        kind_error(ctx, "\"terminals\" must name 2 different pins");
        return NULL;
    }
    for (i = 0; i < pin_count; i++) {
        if (i == bound[0] || i == bound[1]) {
            if (pins[i].role != PIN_INOUT) {
                kind_error(ctx, "crystal pin \"%s\" must be inout", pins[i].name);
                return NULL;
            }
        } else if (pins[i].role != PIN_NC) {
            kind_error(ctx, "crystal pin \"%s\" must be nc", pins[i].name);
            return NULL;
        }
    }
    return bound;
}

/* A number above 0 and at most max, as an integer of `unit` per 1 (llround(v * unit)). */
static int crystal_scaled(KindContext *ctx, const cJSON *json, const char *key, double max,
                          double unit, uint64_t *out) {
    const cJSON *item = kind_member(ctx, json, key, 0);

    if (item == NULL) {
        *out = 0;
        return 0;
    }
    if (!cJSON_IsNumber(item) || !(item->valuedouble > 0) || item->valuedouble > max) {
        return kind_error(ctx, "\"%s\" must be above 0 and at most %.0f", key, max);
    }
    *out = (uint64_t)llround(item->valuedouble * unit);
    return 0;
}

static int crystal_load_part(KindContext *ctx, const void *raw, const void *bound,
                             const cJSON *json, Part *out) {
    const CrystalDevice *device = raw;
    PartCrystal *info = kind_alloc(ctx, sizeof(*info));
    uint64_t tolerance;
    uint64_t stability;
    const cJSON *capacitors;

    (void)bound;
    if (info == NULL) {
        return -1;
    }
    info->type = device->type;
    if (kind_uint(ctx, json, "hz", 1000000000ULL, &info->hz) != 0) {
        return -1;
    }
    if (info->hz < 1) {
        return kind_error(ctx, "\"hz\" must be at least 1");
    }
    if (crystal_scaled(ctx, json, "loadPf", 1000, 1000.0, &info->load_ff) != 0 ||
        crystal_scaled(ctx, json, "tolerancePpm", 100000, 1.0, &tolerance) != 0 ||
        crystal_scaled(ctx, json, "stabilityPpm", 100000, 1.0, &stability) != 0 ||
        crystal_scaled(ctx, json, "esrOhms", 1000000, 1000.0, &info->esr_mohm) != 0) {
        return -1;
    }
    info->tolerance_ppm = (uint32_t)tolerance;
    info->stability_ppm = (uint32_t)stability;
    capacitors = kind_member(ctx, json, "capacitors", 0);
    if (capacitors != NULL) {
        if (kind_bool(ctx, json, "capacitors", &info->capacitors) != 0) {
            return -1;
        }
        if (info->capacitors && strcmp(device->type, "resonator") != 0) {
            return kind_error(ctx, "\"capacitors\" is only for a resonator");
        }
    }
    out->crystal = info;
    return 0;
}

/* ---- Model --------------------------------------------------------------- */

typedef struct {
    size_t x1;
    size_t x2;
    SimTime half_period;
    LogicValue level;
} CrystalState;

static size_t crystal_pin_count(const void *config) {
    return ((const Part *)config)->pin_count;
}

static void *crystal_create(const void *config) {
    const Part *part = config;
    const size_t *pins = part_internal(part)->bound;
    uint64_t hz = part->crystal->hz;
    CrystalState *state;

    if (hz == 0) {
        return NULL;
    }
    state = logic_malloc(sizeof(*state));
    if (state == NULL) {
        return NULL;
    }
    state->x1 = pins[0];
    state->x2 = pins[1];
    state->half_period = (1000000000000ULL + hz) / (2 * hz);
    state->level = LOGIC_0;
    return state;
}

static void crystal_destroy(void *state) {
    logic_free(state);
}

static void drive_terminals(Sim *sim, SimInstance *instance, const CrystalState *state) {
    LogicValue complement = state->level == LOGIC_0 ? LOGIC_1 : LOGIC_0;

    sim_schedule_drive(sim, instance, state->x1, state->level, DRIVE_WEAK, 0);
    sim_schedule_drive(sim, instance, state->x2, complement, DRIVE_WEAK, 0);
}

static void crystal_start(Sim *sim, SimInstance *instance, void *raw) {
    CrystalState *state = raw;

    state->level = LOGIC_0;
    drive_terminals(sim, instance, state);
    sim_schedule_timer(sim, instance, 0, state->half_period);
}

static void crystal_timer(Sim *sim, SimInstance *instance, void *raw, uint32_t timer_id) {
    CrystalState *state = raw;

    (void)timer_id;
    state->level = state->level == LOGIC_0 ? LOGIC_1 : LOGIC_0;
    drive_terminals(sim, instance, state);
    sim_schedule_timer(sim, instance, 0, state->half_period);
}

static const SimComponentType CRYSTAL_MODEL = {
    .name = "crystal",
    .pin_count = crystal_pin_count,
    .create = crystal_create,
    .destroy = crystal_destroy,
    .start = crystal_start,
    .timer = crystal_timer,
};

const PartKindOps CRYSTAL_KIND = {
    PART_CRYSTAL, "crystal", &CRYSTAL_MODEL, DEVICE_KEYS, PART_KEYS,
    crystal_load_device, crystal_bind_pinout, crystal_load_part
};
