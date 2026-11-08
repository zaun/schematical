/*
 * header: a generic pin header (plan-23-A). Passive terminals: each pin is
 * just its net, so the model drives nothing and has no hooks. The device
 * names its row count and the genders a placed header may have; the
 * genders are for the app only (they change the symbol, not the circuit).
 */

#include "kind.h"

static const char *const DEVICE_KEYS[] = {"rows", "genders", NULL};

static void *header_load_device(KindContext *ctx, const cJSON *json) {
    PartHeader *device = kind_alloc(ctx, sizeof(*device));
    uint64_t rows;

    if (device == NULL || kind_uint(ctx, json, "rows", 1000, &rows) != 0) {
        return NULL;
    }
    if (rows != 1 && rows != 2) {
        kind_error(ctx, "\"rows\" must be 1 or 2");
        return NULL;
    }
    device->rows = (unsigned)rows;
    if (kind_names(ctx, json, "genders", &device->genders, &device->gender_count) != 0) {
        return NULL;
    }
    if (device->gender_count == 0) {
        kind_error(ctx, "\"genders\" must not be empty");
        return NULL;
    }
    return device;
}

static void *header_bind_pinout(KindContext *ctx, const void *raw, const PartPin *pins,
                                size_t pin_count) {
    const PartHeader *device = raw;

    (void)pins;
    if (pin_count == 0 || pin_count % device->rows != 0) {
        kind_error(ctx, "a header's pin count must be a multiple of its rows");
        return NULL;
    }
    return (void *)device;
}

static int header_load_part(KindContext *ctx, const void *device, const void *bound,
                            const cJSON *json, Part *out) {
    (void)ctx;
    (void)bound;
    (void)json;
    out->header = device;
    return 0;
}

static size_t header_pin_count(const void *config) { return ((const Part *)config)->pin_count; }

/* No state: any non-NULL pointer tells the engine creation worked. */
static void *header_create(const void *config) { return (void *)((const Part *)config)->header; }

static const SimComponentType HEADER_MODEL = {
    "header", header_pin_count, header_create, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL
};

const PartKindOps HEADER_KIND = {
    PART_HEADER, "header", &HEADER_MODEL, DEVICE_KEYS, NULL,
    header_load_device, header_bind_pinout, header_load_part
};
