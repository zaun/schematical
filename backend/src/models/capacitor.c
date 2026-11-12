/*
 * capacitor (plan-33-E): ceramic, aluminum electrolytic, aluminum polymer,
 * tantalum, tantalum polymer and niobium oxide capacitors. Passive: a
 * capacitor drives nothing and has no analog state in the logic engine. The
 * one behavior is the voltage check's rating test (voltage.c,
 * VOLTAGE_CAPACITOR_OVER): a pin's net above the part's "volts".
 */

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "kind.h"
#include "model_pins.h"

static const char *const DEVICE_KEYS[] = {"type", "farads", "positive", "negative", NULL};
static const char *const PART_KEYS[] = {"volts", "dielectric", "tolerance", NULL};

static const char *const TYPES[] = {
    "ceramic", "aluminum", "aluminum-polymer", "tantalum", "tantalum-polymer",
    "niobium-oxide", NULL
};

typedef struct {
    PartCapacitor info;
    const char *positive;  /* NULL for ceramic */
    const char *negative;
} CapacitorDevice;

static void *capacitor_load_device(KindContext *ctx, const cJSON *json) {
    CapacitorDevice *device = kind_alloc(ctx, sizeof(*device));
    const cJSON *pos;
    const cJSON *neg;
    double farads;
    size_t i;

    if (device == NULL || kind_string(ctx, json, "type", &device->info.type) != 0) {
        return NULL;
    }
    for (i = 0; TYPES[i] != NULL && strcmp(TYPES[i], device->info.type) != 0; i++) {
    }
    if (TYPES[i] == NULL) {
        kind_error(ctx, "unknown capacitor type \"%s\"", device->info.type);
        return NULL;
    }
    device->info.type = TYPES[i];
    device->info.polarized = i != 0;
    if (kind_number(ctx, json, "farads", &farads) != 0) {
        return NULL;
    }
    if (!(farads > 0) || farads > 10) {
        kind_error(ctx, "\"farads\" must be above 0 and at most 10");
        return NULL;
    }
    device->info.capacitance_ff = (uint64_t)llround(farads * 1e15);
    pos = kind_member(ctx, json, "positive", 0);
    neg = kind_member(ctx, json, "negative", 0);
    if (!device->info.polarized) {
        if (pos != NULL || neg != NULL) {
            kind_error(ctx, "a ceramic capacitor has no polarity");
            return NULL;
        }
        return device;
    }
    if (kind_string(ctx, json, "positive", &device->positive) != 0 ||
        kind_string(ctx, json, "negative", &device->negative) != 0) {
        return NULL;
    }
    return device;
}

static void *capacitor_bind_pinout(KindContext *ctx, const void *raw, const PartPin *pins,
                                   size_t pin_count) {
    const CapacitorDevice *device = raw;
    size_t i;
    size_t *bound;

    if (pin_count != 2) {
        kind_error(ctx, "a capacitor has 2 pins");
        return NULL;
    }
    for (i = 0; i < pin_count; i++) {
        if (pins[i].role != PIN_INPUT) {
            kind_error(ctx, "capacitor pin \"%s\" must be input", pins[i].name);
            return NULL;
        }
    }
    bound = kind_alloc(ctx, 2 * sizeof(*bound));
    if (bound == NULL) {
        return NULL;
    }
    bound[0] = 0;
    bound[1] = 1;
    if (device->info.polarized &&
        (kind_pin(ctx, pins, pin_count, device->positive, &bound[0]) != 0 ||
         kind_pin(ctx, pins, pin_count, device->negative, &bound[1]) != 0)) {
        return NULL;
    }
    return bound;
}

static int capacitor_load_part(KindContext *ctx, const void *raw, const void *bound,
                               const cJSON *json, Part *out) {
    const CapacitorDevice *device = raw;
    PartCapacitor *info = kind_alloc(ctx, sizeof(*info));
    const cJSON *volts;
    const cJSON *dielectric;
    const cJSON *tolerance;

    (void)bound;
    if (info == NULL) {
        return -1;
    }
    *info = device->info;
    info->rated_mv = 0;
    info->dielectric = NULL;
    info->tolerance = NULL;
    volts = kind_member(ctx, json, "volts", 0);
    if (volts != NULL) {
        if (!cJSON_IsNumber(volts) || !(volts->valuedouble > 0) || volts->valuedouble > 10000) {
            return kind_error(ctx, "\"volts\" must be above 0 and at most 10000");
        }
        info->rated_mv = (uint32_t)llround(volts->valuedouble * 1000);
    }
    dielectric = kind_member(ctx, json, "dielectric", 0);
    if (dielectric != NULL) {
        if (device->info.polarized) {
            return kind_error(ctx, "\"dielectric\" is only for a ceramic capacitor");
        }
        if (kind_string(ctx, json, "dielectric", &info->dielectric) != 0) {
            return -1;
        }
    }
    tolerance = kind_member(ctx, json, "tolerance", 0);
    if (tolerance != NULL && kind_string(ctx, json, "tolerance", &info->tolerance) != 0) {
        return -1;
    }
    out->capacitor = info;
    return 0;
}

/* ---- Model --------------------------------------------------------------- */

static size_t capacitor_pin_count(const void *config) {
    return ((const Part *)config)->pin_count;
}

/* No state: any non-NULL pointer tells the engine creation worked. */
static void *capacitor_create(const void *config) {
    return (void *)((const Part *)config)->capacitor;
}

static const SimComponentType CAPACITOR_MODEL = {
    .name = "capacitor",
    .pin_count = capacitor_pin_count,
    .create = capacitor_create,
};

const PartKindOps CAPACITOR_KIND = {
    PART_CAPACITOR, "capacitor", &CAPACITOR_MODEL, DEVICE_KEYS, PART_KEYS,
    capacitor_load_device, capacitor_bind_pinout, capacitor_load_part
};
