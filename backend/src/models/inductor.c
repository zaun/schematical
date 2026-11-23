/*
 * inductor (plan-47-I): fixed two-terminal inductors. In the logic
 * simulation an inductor conducts like a wire: its two terminals are joined
 * by an engine contact (sim_set_contact) that is closed from the start and
 * never opens, so the nets on both terminals act as one net and a gate
 * driving one terminal drives the other. Every further pin of a terminal's
 * name (a four-pad part has two pads per terminal) is tied to the terminal's
 * first pin the same way. The facts (inductance, tolerance, currents, DC
 * resistance, shielding) are data for the app; nothing in the simulation
 * reads them: no inductance, no current, no flyback.
 */

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "kind.h"
#include "model_pins.h"
#include "terminals.h"

static const char *const DEVICE_KEYS[] = {"terminals", NULL};
static const char *const PART_KEYS[] = {"henries", "tolerance", "amps", "isat", "dcr", "shielding",
                                         NULL};
static const char *const SHIELDING[] = {"shielded", "semi-shielded", "unshielded", NULL};

typedef struct {
    const char *terminals[2];
} InductorDevice;

static void *inductor_load_device(KindContext *ctx, const cJSON *json) {
    InductorDevice *device = kind_alloc(ctx, sizeof(*device));

    if (device == NULL || terminals_names(ctx, json, device->terminals) != 0) {
        return NULL;
    }
    return device;
}

static void *inductor_bind_pinout(KindContext *ctx, const void *raw, const PartPin *pins,
                                  size_t pin_count) {
    const InductorDevice *device = raw;

    return terminals_bind(ctx, "inductor", device->terminals, pins, pin_count);
}

/* An optional number above 0 and at most max, scaled by `unit` and rounded; 0 when absent. */
static int optional_scaled(KindContext *ctx, const cJSON *json, const char *key, double max,
                           double unit, uint64_t *out) {
    const cJSON *item = kind_member(ctx, json, key, 0);

    *out = 0;
    if (item == NULL) {
        return 0;
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

static int inductor_load_part(KindContext *ctx, const void *raw, const void *bound,
                              const cJSON *json, Part *out) {
    PartInductor *info = kind_alloc(ctx, sizeof(*info));
    const cJSON *item;
    uint64_t value;
    size_t i;

    (void)raw;
    (void)bound;
    if (info == NULL) {
        return -1;
    }
    item = kind_member(ctx, json, "henries", 1);
    if (item == NULL) {
        return -1;
    }
    if (!cJSON_IsNumber(item) || !(item->valuedouble > 0) || item->valuedouble > 100) {
        return kind_error(ctx, "\"henries\" must be above 0 and at most 100");
    }
    info->ph = (uint64_t)llround(item->valuedouble * 1e12);
    if (info->ph == 0) {
        info->ph = 1;
    }
    if (kind_member(ctx, json, "tolerance", 0) != NULL &&
        kind_string(ctx, json, "tolerance", &info->tolerance) != 0) {
        return -1;
    }
    if (optional_scaled(ctx, json, "amps", 1000, 1000.0, &value) != 0) {
        return -1;
    }
    info->rated_ma = (uint32_t)value;
    if (optional_scaled(ctx, json, "isat", 1000, 1000.0, &value) != 0) {
        return -1;
    }
    info->isat_ma = (uint32_t)value;
    if (optional_scaled(ctx, json, "dcr", 100000, 1e6, &info->dcr_uohm) != 0) {
        return -1;
    }
    if (kind_member(ctx, json, "shielding", 0) != NULL) {
        const char *text;

        if (kind_string(ctx, json, "shielding", &text) != 0) {
            return -1;
        }
        for (i = 0; SHIELDING[i] != NULL && strcmp(SHIELDING[i], text) != 0; i++) {
        }
        if (SHIELDING[i] == NULL) {
            return kind_error(ctx, "unknown shielding \"%s\"", text);
        }
        info->shielding = SHIELDING[i];
    }
    out->inductor = info;
    return 0;
}

int inductor_fixed_contact(const Part *part, size_t k, size_t *a, size_t *b) {
    return terminals_contact(part_internal(part)->bound, k, a, b);
}

/* ---- Model --------------------------------------------------------------- */

static size_t inductor_pin_count(const void *config) {
    return ((const Part *)config)->pin_count;
}

/* The state is the part's terminals (library memory, alive while the part is). */
static void *inductor_create(const void *config) {
    return (void *)part_internal(config)->bound;
}

static void inductor_start(Sim *sim, SimInstance *instance, void *raw) {
    const TerminalPins *t = raw;

    sim_set_contact(sim, instance, t->a, t->b, 1);
    terminals_close_ties(sim, instance, t);
}

static const SimComponentType INDUCTOR_MODEL = {
    .name = "inductor",
    .pin_count = inductor_pin_count,
    .create = inductor_create,
    .start = inductor_start,
};

const PartKindOps INDUCTOR_KIND = {
    PART_INDUCTOR, "inductor", &INDUCTOR_MODEL, DEVICE_KEYS, PART_KEYS,
    inductor_load_device, inductor_bind_pinout, inductor_load_part
};
