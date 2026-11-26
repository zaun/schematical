/*
 * fuse (plan-47-I): board-mount fuses ("type": "fuse") and PTC resettable
 * fuses ("type": "ptc"). In the logic simulation an intact fuse conducts like
 * a wire, as an inductor does (inductor.c): its two terminals are joined by
 * an engine contact. The contact is the switch position 1 of the part:
 * circuit_set_switch(part, 1, 0) blows the fuse (the contact opens) and
 * circuit_set_switch(part, 1, 1) restores it; a fuse blown before the run
 * starts open. A fuse never blows by itself (the logic simulation has no
 * current) and a PTC never trips. The ties between pins of one terminal's name
 * stay closed whatever the fuse does: they are one piece of metal. The facts
 * (currents, ratings, speed) are data for the app, except the voltage check's
 * rating test (voltage.c, VOLTAGE_FUSE_OVER).
 */

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "kind.h"
#include "model_pins.h"
#include "terminals.h"
#include "logic_alloc.h"

static const char *const DEVICE_KEYS[] = {"type", "terminals", NULL};
static const char *const PART_KEYS[] = {"amps", "speed", "voltsAc", "voltsDc", "breaking",
                                         "hold", "trip", "volts", "imax", NULL};
static const char *const TYPES[] = {"fuse", "ptc", NULL};
static const char *const SPEEDS[] = {"fast", "medium", "slow", NULL};

/* The keys of each type; a key of the other one is an error. */
static const char *const FUSE_ONLY[] = {"amps", "speed", "voltsAc", "voltsDc", "breaking", NULL};
static const char *const PTC_ONLY[] = {"hold", "trip", "volts", "imax", NULL};

typedef struct {
    const char *type;
    const char *terminals[2];
} FuseDevice;

static void *fuse_load_device(KindContext *ctx, const cJSON *json) {
    FuseDevice *device = kind_alloc(ctx, sizeof(*device));
    size_t i;

    if (device == NULL || kind_string(ctx, json, "type", &device->type) != 0) {
        return NULL;
    }
    for (i = 0; TYPES[i] != NULL && strcmp(TYPES[i], device->type) != 0; i++) {
    }
    if (TYPES[i] == NULL) {
        kind_error(ctx, "unknown fuse type \"%s\"", device->type);
        return NULL;
    }
    device->type = TYPES[i];
    if (terminals_names(ctx, json, device->terminals) != 0) {
        return NULL;
    }
    return device;
}

static void *fuse_bind_pinout(KindContext *ctx, const void *raw, const PartPin *pins,
                              size_t pin_count) {
    const FuseDevice *device = raw;

    return terminals_bind(ctx, "fuse", device->terminals, pins, pin_count);
}

/* A number above 0 and at most max, scaled by `unit` and rounded (at least 1). */
static int scaled(KindContext *ctx, const cJSON *json, const char *key, int required, double max,
                  double unit, uint64_t *out) {
    const cJSON *item = kind_member(ctx, json, key, required);

    *out = 0;
    if (item == NULL) {
        return required ? -1 : 0;
    }
    if (!cJSON_IsNumber(item) || !(item->valuedouble > 0) || item->valuedouble > max) {
        return kind_error(ctx, "\"%s\" must be above 0 and at most %g", key, max);
    }
    *out = (uint64_t)llround(item->valuedouble * unit);
    if (*out == 0) {
        *out = 1;
    }
    return 0;
}

static int has_key(const cJSON *json, const char *key) {
    return cJSON_GetObjectItemCaseSensitive(json, key) != NULL;
}

/* A key of the other type is an error naming the type it is for. */
static int check_type_keys(KindContext *ctx, const cJSON *json, int is_ptc) {
    const char *const *other = is_ptc ? FUSE_ONLY : PTC_ONLY;
    size_t i;

    for (i = 0; other[i] != NULL; i++) {
        if (has_key(json, other[i])) {
            return is_ptc ? kind_error(ctx, "\"%s\" is not a ptc key", other[i])
                          : kind_error(ctx, "\"%s\" is only for a ptc", other[i]);
        }
    }
    return 0;
}

static int fuse_load_part(KindContext *ctx, const void *raw, const void *bound, const cJSON *json,
                          Part *out) {
    const FuseDevice *device = raw;
    PartFuse *info = kind_alloc(ctx, sizeof(*info));
    int is_ptc = strcmp(device->type, "ptc") == 0;
    uint64_t amps = 0, hold = 0, trip = 0, ac = 0, dc = 0, breaking = 0;

    (void)bound;
    if (info == NULL) {
        return -1;
    }
    info->type = device->type;
    if (check_type_keys(ctx, json, is_ptc) != 0) {
        return -1;
    }
    if (!is_ptc) {
        const char *speed;
        size_t i;

        if (scaled(ctx, json, "amps", 1, 1000, 1000.0, &amps) != 0 ||
            kind_string(ctx, json, "speed", &speed) != 0) {
            return -1;
        }
        for (i = 0; SPEEDS[i] != NULL && strcmp(SPEEDS[i], speed) != 0; i++) {
        }
        if (SPEEDS[i] == NULL) {
            return kind_error(ctx, "unknown fuse speed \"%s\"", speed);
        }
        info->speed = SPEEDS[i];
        if (!has_key(json, "voltsAc") && !has_key(json, "voltsDc")) {
            return kind_error(ctx, "a fuse needs \"voltsAc\" or \"voltsDc\"");
        }
        if (scaled(ctx, json, "voltsAc", 0, 100000, 1000.0, &ac) != 0 ||
            scaled(ctx, json, "voltsDc", 0, 100000, 1000.0, &dc) != 0 ||
            scaled(ctx, json, "breaking", 0, 1000000, 1.0, &breaking) != 0) {
            return -1;
        }
        info->rated_ma = (uint32_t)amps;
    } else {
        if (scaled(ctx, json, "hold", 1, 1000, 1000.0, &hold) != 0 ||
            scaled(ctx, json, "trip", 1, 1000, 1000.0, &trip) != 0 ||
            scaled(ctx, json, "volts", 1, 100000, 1000.0, &dc) != 0 ||
            scaled(ctx, json, "imax", 0, 1000000, 1.0, &breaking) != 0) {
            return -1;
        }
        if (trip <= hold) {
            return kind_error(ctx, "\"trip\" must be above \"hold\"");
        }
        info->rated_ma = (uint32_t)hold;
        info->trip_ma = (uint32_t)trip;
        info->speed = NULL;
    }
    info->ac_mv = (uint32_t)ac;
    info->dc_mv = (uint32_t)dc;
    info->breaking_a = (uint32_t)breaking;
    out->fuse = info;
    return 0;
}

int fuse_fixed_contact(const Part *part, size_t k, size_t *a, size_t *b) {
    /* model_pins.h: 0 for a part of another kind (its bound is no TerminalPins). */
    if (part == NULL || part->kind != PART_FUSE) {
        return 0;
    }
    return terminals_contact(part_internal(part)->bound, k, a, b);
}

/* ---- Model --------------------------------------------------------------- */

typedef struct {
    const TerminalPins *terminals;
    int intact;
} FuseState;

static size_t fuse_pin_count(const void *config) {
    return ((const Part *)config)->pin_count;
}

static void *fuse_create(const void *config) {
    FuseState *state = logic_malloc(sizeof(*state));

    if (state == NULL) {
        return NULL;
    }
    state->terminals = part_internal(config)->bound;
    state->intact = 1;
    return state;
}

static void fuse_destroy(void *state) {
    logic_free(state);
}

static void fuse_start(Sim *sim, SimInstance *instance, void *raw) {
    FuseState *state = raw;

    contacts_set(sim, instance, &state->terminals->main, state->intact);
    contacts_set(sim, instance, &state->terminals->ties, 1);
}

/* Position 1 is the fuse: on = intact, off = blown. */
static int fuse_set(Sim *sim, SimInstance *instance, void *raw, size_t position, int on) {
    FuseState *state = raw;

    if (position != 1) {
        return 0;
    }
    state->intact = on != 0;
    contacts_set(sim, instance, &state->terminals->main, state->intact);
    return 1;
}

static const SimComponentType FUSE_MODEL = {
    .name = "fuse",
    .pin_count = fuse_pin_count,
    .create = fuse_create,
    .destroy = fuse_destroy,
    .start = fuse_start,
    .set_switch = fuse_set,
};

const PartKindOps FUSE_KIND = {
    PART_FUSE, "fuse", &FUSE_MODEL, DEVICE_KEYS, PART_KEYS,
    fuse_load_device, fuse_bind_pinout, fuse_load_part
};
