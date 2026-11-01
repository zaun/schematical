/*
 * supply: constant sources. A supply of more than 0 V drives its one pin to
 * 1, GND (0 V) to 0, both strong, for the whole run (ported from
 * backend/supply.c). The voltage itself is only for the voltage check.
 */

#include "kind.h"

static const char *const DEVICE_KEYS[] = {"volts", NULL};

typedef struct {
    PartSupply supply;
} SupplyDevice;

static void *supply_load_device(KindContext *ctx, const cJSON *json) {
    SupplyDevice *device = kind_alloc(ctx, sizeof(*device));
    if (device == NULL || kind_volts(ctx, json, "volts", 30, &device->supply.mv) != 0) {
        return NULL;
    }
    device->supply.level = device->supply.mv > 0;
    return device;
}

static void *supply_bind_pinout(KindContext *ctx, const void *device, const PartPin *pins,
                                size_t pin_count) {
    (void)pins;
    if (pin_count != 1) {
        kind_error(ctx, "a supply has exactly one pin");
        return NULL;
    }
    return (void *)device;
}

static int supply_load_part(KindContext *ctx, const void *device, const void *bound,
                            const cJSON *json, Part *out) {
    (void)ctx;
    (void)bound;
    (void)json;
    out->supply = &((const SupplyDevice *)device)->supply;
    return 0;
}

/* ---- Model: config and state are the Part --------------------------------- */

static size_t supply_pin_count(const void *config) {
    (void)config;
    return 1;
}

static void *supply_create(const void *config) {
    return (void *)config;
}

static void supply_start(Sim *sim, SimInstance *instance, void *state) {
    const Part *part = state;

    sim_schedule_drive(sim, instance, 0, part->supply->level ? LOGIC_1 : LOGIC_0,
                       DRIVE_STRONG, 0);
}

static const SimComponentType SUPPLY_MODEL = {
    "supply", supply_pin_count, supply_create, NULL, supply_start, NULL, NULL,
    NULL, NULL, NULL, NULL
};

const PartKindOps SUPPLY_KIND = {
    PART_SUPPLY, "supply", &SUPPLY_MODEL, DEVICE_KEYS, NULL,
    supply_load_device, supply_bind_pinout, supply_load_part
};
