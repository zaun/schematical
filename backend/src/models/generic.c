/*
 * generic: a part that is only a symbol and a footprint (plan-46-B). Facts
 * only, and fewer than an mcu's: pins, a footprint and an optional supply
 * range for the voltage check. No key names a pin and there is no model, so
 * every generic device must say "simulated": false and circuit_add_part
 * refuses its parts. The engine model is passive (the header.c shape) so
 * that trace_check's bare engine harness needs no special case.
 */

#include "kind.h"

static const char *const DEVICE_KEYS[] = {"vcc", "inputMax", NULL};
static const char *const PART_KEYS[] = {"vcc", "inputMax", NULL};

typedef struct {
    const PartVoltage *vcc;   /* NULL = the device names no range */
} GenericDevice;

static void *generic_load_device(KindContext *ctx, const cJSON *json) {
    GenericDevice *device = kind_alloc(ctx, sizeof(*device));

    if (device == NULL) {
        return NULL;
    }
    if (!cJSON_IsFalse(cJSON_GetObjectItemCaseSensitive(json, "simulated"))) {
        kind_error(ctx, "a generic part has no simulation model: \"simulated\" must be false");
        return NULL;
    }
    if (kind_voltage(ctx, json, NULL, &device->vcc) != 0) {
        return NULL;
    }
    return device;
}

static void *generic_bind_pinout(KindContext *ctx, const void *raw, const PartPin *pins,
                                 size_t pin_count) {
    (void)pins;
    if (pin_count == 0) {
        kind_error(ctx, "a generic part needs at least one pin");
        return NULL;
    }
    return (void *)raw;
}

static int generic_load_part(KindContext *ctx, const void *raw, const void *bound,
                             const cJSON *json, Part *out) {
    const GenericDevice *device = raw;

    (void)bound;
    /* A supply range is optional: a part has one only when the device or the
       part names it. kind_part_voltage would demand one, so this is
       kind_voltage, which leaves the range NULL when neither does. */
    return kind_voltage(ctx, json, device->vcc, &out->vcc);
}

static size_t generic_pin_count(const void *config) { return ((const Part *)config)->pin_count; }

/* No state and no info struct: the Part itself is the non-NULL pointer that
   tells the engine creation worked. */
static void *generic_create(const void *config) { return (void *)config; }

static const SimComponentType GENERIC_MODEL = {
    "generic", generic_pin_count, generic_create, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL
};

const PartKindOps GENERIC_KIND = {
    PART_GENERIC, "generic", &GENERIC_MODEL, DEVICE_KEYS, PART_KEYS,
    generic_load_device, generic_bind_pinout, generic_load_part
};
