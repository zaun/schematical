/*
 * mcu: a microcontroller (plan-36-F). Facts only: core, clock, memory
 * sizes and supply range, for the catalog, the BOM and the voltage check.
 * There is no model of the core or its peripherals, so every mcu device
 * must say "simulated": false and circuit_add_part refuses its parts. The
 * engine model is passive (the header.c shape) so that trace_check's bare
 * engine harness, the one place that instantiates it, needs no special
 * case.
 */

#include <string.h>

#include "kind.h"

static const char *const DEVICE_KEYS[] = {"core", "clock", "flash", "ram", "eeprom", "vcc",
                                          "inputMax", NULL};
static const char *const PART_KEYS[] = {"vcc", "inputMax", "clock", "flash", "psram", NULL};

static const char *const CORES[] = {"avr", "avrrc", "avrxt", "avrxm", "cortex-m0plus",
                                    "xtensa-lx6", "xtensa-lx7", "riscv", NULL};

typedef struct {
    PartMcu facts;
    const PartVoltage *vcc;
} McuDevice;

/* An optional byte count (at most 4294967295) into *out; absent leaves it. */
static int mcu_bytes(KindContext *ctx, const cJSON *json, const char *key, uint32_t *out) {
    uint64_t value;

    if (cJSON_GetObjectItemCaseSensitive(json, key) == NULL) {
        return 0;
    }
    if (kind_uint(ctx, json, key, UINT32_MAX, &value) != 0) {
        return -1;
    }
    *out = (uint32_t)value;
    return 0;
}

/* An optional clock (Hz, above 0) into *out; absent leaves it. */
static int mcu_clock(KindContext *ctx, const cJSON *json, uint64_t *out) {
    uint64_t value;

    if (cJSON_GetObjectItemCaseSensitive(json, "clock") == NULL) {
        return 0;
    }
    if (kind_uint(ctx, json, "clock", UINT64_MAX, &value) != 0) {
        return -1;
    }
    if (value == 0) {
        return kind_error(ctx, "\"clock\" must be above 0");
    }
    *out = value;
    return 0;
}

static void *mcu_load_device(KindContext *ctx, const cJSON *json) {
    McuDevice *device = kind_alloc(ctx, sizeof(*device));
    const char *core;
    uint64_t value;
    size_t i;

    if (device == NULL) {
        return NULL;
    }
    if (!cJSON_IsFalse(cJSON_GetObjectItemCaseSensitive(json, "simulated"))) {
        kind_error(ctx, "an mcu has no simulation model: \"simulated\" must be false");
        return NULL;
    }
    if (kind_string(ctx, json, "core", &core) != 0) {
        return NULL;
    }
    for (i = 0; CORES[i] != NULL && strcmp(CORES[i], core) != 0; i++) {
    }
    if (CORES[i] == NULL) {
        kind_error(ctx, "unknown mcu core \"%s\"", core);
        return NULL;
    }
    device->facts.core = CORES[i];
    if (kind_member(ctx, json, "clock", 1) == NULL || mcu_clock(ctx, json, &device->facts.max_hz) != 0 ||
        kind_uint(ctx, json, "flash", UINT32_MAX, &value) != 0) {
        return NULL;
    }
    device->facts.flash = (uint32_t)value;
    if (kind_uint(ctx, json, "ram", UINT32_MAX, &value) != 0) {
        return NULL;
    }
    device->facts.ram = (uint32_t)value;
    if (mcu_bytes(ctx, json, "eeprom", &device->facts.eeprom) != 0 ||
        kind_voltage(ctx, json, NULL, &device->vcc) != 0) {
        return NULL;
    }
    return device;
}

static void *mcu_bind_pinout(KindContext *ctx, const void *raw, const PartPin *pins,
                             size_t pin_count) {
    int power = 0, ground = 0;
    size_t i;

    for (i = 0; i < pin_count; i++) {
        power |= pins[i].role == PIN_POWER;
        ground |= pins[i].role == PIN_GROUND;
    }
    if (!power) {
        kind_error(ctx, "an mcu pinout needs a power pin");
        return NULL;
    }
    if (!ground) {
        kind_error(ctx, "an mcu pinout needs a ground pin");
        return NULL;
    }
    return (void *)raw;
}

static int mcu_load_part(KindContext *ctx, const void *raw, const void *bound,
                         const cJSON *json, Part *out) {
    const McuDevice *device = raw;
    PartMcu *info = kind_alloc(ctx, sizeof(*info));

    (void)bound;
    if (info == NULL) {
        return -1;
    }
    *info = device->facts;
    if (mcu_clock(ctx, json, &info->max_hz) != 0 || mcu_bytes(ctx, json, "flash", &info->flash) != 0 ||
        mcu_bytes(ctx, json, "psram", &info->psram) != 0) {
        return -1;
    }
    out->mcu = info;
    return kind_part_voltage(ctx, json, device->vcc, &out->vcc);
}

static size_t mcu_pin_count(const void *config) { return ((const Part *)config)->pin_count; }

/* No state: any non-NULL pointer tells the engine creation worked. */
static void *mcu_create(const void *config) { return (void *)((const Part *)config)->mcu; }

static const SimComponentType MCU_MODEL = {
    "mcu", mcu_pin_count, mcu_create, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL
};

const PartKindOps MCU_KIND = {
    PART_MCU, "mcu", &MCU_MODEL, DEVICE_KEYS, PART_KEYS,
    mcu_load_device, mcu_bind_pinout, mcu_load_part
};
