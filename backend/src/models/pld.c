/*
 * pld: a simple or complex programmable logic device (GAL16V8, ATF22V10,
 * ATF750C, ...). Facts only for now: the macrocell count, the speed grade
 * (the pin-to-pin delay) and the supply range, for the catalog, the BOM and
 * the voltage check. What the chip does is the user's program, and there is
 * no model of it yet, so every pld device must say "simulated": false and
 * circuit_add_part refuses its parts. The engine model is passive (the
 * header.c shape) so that trace_check's bare engine harness, the one place
 * that instantiates it, needs no special case.
 */

#include "kind.h"

static const char *const DEVICE_KEYS[] = {"macrocells", "vcc", "inputMax", NULL};
static const char *const PART_KEYS[] = {"tpd", "vcc", "inputMax", NULL};

typedef struct {
    uint32_t macrocells;
    const PartVoltage *vcc;
} PldDevice;

static void *pld_load_device(KindContext *ctx, const cJSON *json) {
    PldDevice *device = kind_alloc(ctx, sizeof(*device));
    uint64_t value;

    if (device == NULL) {
        return NULL;
    }
    if (!cJSON_IsFalse(cJSON_GetObjectItemCaseSensitive(json, "simulated"))) {
        kind_error(ctx, "a pld has no simulation model: \"simulated\" must be false");
        return NULL;
    }
    if (kind_uint(ctx, json, "macrocells", 1024, &value) != 0) {
        return NULL;
    }
    if (value == 0) {
        kind_error(ctx, "\"macrocells\" must be above 0");
        return NULL;
    }
    device->macrocells = (uint32_t)value;
    if (kind_voltage(ctx, json, NULL, &device->vcc) != 0) {
        return NULL;
    }
    return device;
}

static void *pld_bind_pinout(KindContext *ctx, const void *raw, const PartPin *pins,
                             size_t pin_count) {
    int power = 0, ground = 0;
    size_t i;

    for (i = 0; i < pin_count; i++) {
        power |= pins[i].role == PIN_POWER;
        ground |= pins[i].role == PIN_GROUND;
    }
    if (!power) {
        kind_error(ctx, "a pld pinout needs a power pin");
        return NULL;
    }
    if (!ground) {
        kind_error(ctx, "a pld pinout needs a ground pin");
        return NULL;
    }
    return (void *)raw;
}

static int pld_load_part(KindContext *ctx, const void *raw, const void *bound,
                         const cJSON *json, Part *out) {
    const PldDevice *device = raw;
    PartPld *info = kind_alloc(ctx, sizeof(*info));
    uint64_t ps;

    (void)bound;
    if (info == NULL) {
        return -1;
    }
    info->macrocells = device->macrocells;
    if (kind_member(ctx, json, "tpd", 1) == NULL ||
        kind_ns(ctx, cJSON_GetObjectItemCaseSensitive(json, "tpd"), "tpd", &ps) != 0) {
        return -1;
    }
    if (ps == 0) {
        return kind_error(ctx, "\"tpd\" must be above 0");
    }
    info->tpd_ps = ps;
    out->pld = info;
    return kind_part_voltage(ctx, json, device->vcc, &out->vcc);
}

static size_t pld_pin_count(const void *config) { return ((const Part *)config)->pin_count; }

/* No state: any non-NULL pointer tells the engine creation worked. */
static void *pld_create(const void *config) { return (void *)((const Part *)config)->pld; }

static const SimComponentType PLD_MODEL = {
    "pld", pld_pin_count, pld_create, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL
};

const PartKindOps PLD_KIND = {
    PART_PLD, "pld", &PLD_MODEL, DEVICE_KEYS, PART_KEYS,
    pld_load_device, pld_bind_pinout, pld_load_part
};
