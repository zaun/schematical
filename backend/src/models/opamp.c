/*
 * opamp (plan-48-F): operational amplifiers with one, two or four
 * amplifiers. Facts only in the simulation: the model is passive (no
 * callbacks, the capacitor.c shape), so a placed op-amp drives nothing and
 * does not turn Run off (a device with "simulated": false would, for the whole
 * schematic). The voltage check tests its V+ pin against the supply span
 * ("vcc", the single-supply span V+ - V-: the app has no negative supply)
 * and skips every amplifier pin (inputs and outputs are analog).
 */

#include <stdlib.h>
#include <string.h>

#include "kind.h"

static const char *const DEVICE_KEYS[] = {"type", "amplifiers", "gbwHz", "slewVus", "vosMv", "railIn",
                                           "railOut", "vcc", "inputMax", NULL};
static const char *const PART_KEYS[] = {"vcc", "inputMax", "vosMv", NULL};
static const char *const AMPLIFIER_KEYS[] = {"plus", "minus", "output", NULL};
static const char *const TYPES[] = {"general", "jfet", "cmos", NULL};

typedef struct {
    const char *plus, *minus, *output;
} OpampNames;

typedef struct {
    const char *type;
    OpampNames amplifiers[4];
    size_t amplifier_count;
    uint64_t gbw_hz;
    uint32_t slew_mv_us, vos_uv;
    int rail_in, rail_out;
    const PartVoltage *vcc;
} OpampDevice;

/* Bound to a pinout: every amplifier pin, as indices into Part.pins. */
typedef struct {
    size_t analog[12];
    size_t analog_count;
} OpampBound;

/* A number from 0 to max (an absent key is 0), scaled by `unit` and rounded. */
static int scaled(KindContext *ctx, const cJSON *json, const char *key, double max, double unit, uint64_t *out) {
    const cJSON *item = kind_member(ctx, json, key, 0);

    *out = 0;
    if (item == NULL) {
        return 0;
    }
    if (!cJSON_IsNumber(item) || !(item->valuedouble >= 0) || item->valuedouble > max) {
        return kind_error(ctx, "\"%s\" must be from 0 to %g", key, max);
    }
    *out = (uint64_t)(item->valuedouble * unit + 0.5);
    return 0;
}

static void *opamp_load_device(KindContext *ctx, const cJSON *json) {
    OpampDevice *device = kind_alloc(ctx, sizeof(*device));
    const cJSON *amplifiers, *entry;
    const char *text;
    uint64_t value;
    size_t i, k;

    if (device == NULL || kind_string(ctx, json, "type", &text) != 0) {
        return NULL;
    }
    for (i = 0; TYPES[i] != NULL; i++) {
        if (strcmp(TYPES[i], text) == 0) {
            device->type = TYPES[i];
        }
    }
    if (device->type == NULL) {
        kind_error(ctx, "unknown op-amp type \"%s\"", text);
        return NULL;
    }
    amplifiers = kind_member(ctx, json, "amplifiers", 1);
    if (amplifiers == NULL) {
        return NULL;
    }
    i = cJSON_IsArray(amplifiers) ? (size_t)cJSON_GetArraySize(amplifiers) : 0;
    if (i != 1 && i != 2 && i != 4) {
        kind_error(ctx, "\"amplifiers\" must have 1, 2 or 4 entries");
        return NULL;
    }
    device->amplifier_count = i;
    k = 0;
    cJSON_ArrayForEach(entry, amplifiers) {
        OpampNames *names = &device->amplifiers[k];

        kind_push(ctx, "amplifiers[%zu]", k);
        if (!cJSON_IsObject(entry) || kind_check_keys(ctx, entry, AMPLIFIER_KEYS, NULL) != 0 ||
            kind_string(ctx, entry, "plus", &names->plus) != 0 ||
            kind_string(ctx, entry, "minus", &names->minus) != 0 ||
            kind_string(ctx, entry, "output", &names->output) != 0) {
            if (!kind_failed(ctx)) {
                kind_error(ctx, "an amplifier must be an object");
            }
            kind_pop(ctx);
            return NULL;
        }
        kind_pop(ctx);
        k++;
    }
    if (scaled(ctx, json, "gbwHz", 100000000000.0, 1.0, &device->gbw_hz) != 0 ||
        scaled(ctx, json, "slewVus", 100000.0, 1000.0, &value) != 0) {
        return NULL;
    }
    device->slew_mv_us = (uint32_t)value;
    if (scaled(ctx, json, "vosMv", 1000.0, 1000.0, &value) != 0) {
        return NULL;
    }
    device->vos_uv = (uint32_t)value;
    if ((kind_member(ctx, json, "railIn", 0) != NULL && kind_bool(ctx, json, "railIn", &device->rail_in) != 0) ||
        (kind_member(ctx, json, "railOut", 0) != NULL && kind_bool(ctx, json, "railOut", &device->rail_out) != 0)) {
        return NULL;
    }
    if (kind_voltage(ctx, json, NULL, &device->vcc) != 0) {
        return NULL;
    }
    return device;
}

static int bind_name(KindContext *ctx, const PartPin *pins, size_t pin_count, const char *name, PinRole role,
                     const char *role_text, unsigned char *seen, OpampBound *bound) {
    size_t index;

    if (kind_pin(ctx, pins, pin_count, name, &index) != 0) {
        return -1;
    }
    if (seen[index]) {
        return kind_error(ctx, "op-amp pin \"%s\" is named twice", name);
    }
    seen[index] = 1;
    if (pins[index].role != role) {
        return kind_error(ctx, "op-amp pin \"%s\" must be %s", name, role_text);
    }
    bound->analog[bound->analog_count++] = index;
    return 0;
}

static void *opamp_bind_pinout(KindContext *ctx, const void *raw, const PartPin *pins, size_t pin_count) {
    const OpampDevice *device = raw;
    OpampBound *bound = kind_alloc(ctx, sizeof(*bound));
    unsigned char *seen = kind_alloc(ctx, pin_count);
    size_t i, powers = 0, grounds = 0;

    if (bound == NULL || seen == NULL) {
        return NULL;
    }
    for (i = 0; i < device->amplifier_count; i++) {
        const OpampNames *n = &device->amplifiers[i];

        if (bind_name(ctx, pins, pin_count, n->plus, PIN_INPUT, "an input", seen, bound) != 0 ||
            bind_name(ctx, pins, pin_count, n->minus, PIN_INPUT, "an input", seen, bound) != 0 ||
            bind_name(ctx, pins, pin_count, n->output, PIN_OUTPUT, "an output", seen, bound) != 0) {
            return NULL;
        }
    }
    for (i = 0; i < pin_count; i++) {
        powers += pins[i].role == PIN_POWER;
        grounds += pins[i].role == PIN_GROUND;
        if (!seen[i] && pins[i].role != PIN_POWER && pins[i].role != PIN_GROUND && pins[i].role != PIN_NC &&
            pins[i].role != PIN_INPUT && pins[i].role != PIN_OUTPUT && pins[i].role != PIN_ABSENT) {
            kind_error(ctx, "op-amp pin \"%s\" must be an input, an output, ground or nc", pins[i].name);
            return NULL;
        }
    }
    if (powers < 1 || grounds < 1) {
        kind_error(ctx, "an op-amp needs a power pin and a ground pin");
        return NULL;
    }
    return bound;
}

static int opamp_load_part(KindContext *ctx, const void *raw, const void *bound, const cJSON *json, Part *out) {
    const OpampDevice *device = raw;
    PartOpamp *info = kind_alloc(ctx, sizeof(*info));
    uint64_t vos = device->vos_uv;

    (void)bound;
    if (info == NULL) {
        return -1;
    }
    if (kind_member(ctx, json, "vosMv", 0) != NULL && scaled(ctx, json, "vosMv", 1000.0, 1000.0, &vos) != 0) {
        return -1;
    }
    info->type = device->type;
    info->amplifier_count = device->amplifier_count;
    info->rail_in = (uint8_t)device->rail_in;
    info->rail_out = (uint8_t)device->rail_out;
    info->gbw_hz = device->gbw_hz;
    info->slew_mv_us = device->slew_mv_us;
    info->vos_uv = (uint32_t)vos;
    out->opamp = info;
    return kind_part_voltage(ctx, json, device->vcc, &out->vcc);
}

static int opamp_pin_analog(const Part *part, size_t pin) {
    const OpampBound *bound = part_internal(part)->bound;
    size_t i;

    for (i = 0; i < bound->analog_count; i++) {
        if (bound->analog[i] == pin) {
            return 1;
        }
    }
    return 0;
}

/* ---- Model --------------------------------------------------------------- */

static size_t opamp_pin_count(const void *config) {
    return ((const Part *)config)->pin_count;
}

static void *opamp_create(const void *config) {
    return (void *)config;
}

static const SimComponentType OPAMP_MODEL = {
    .name = "opamp",
    .pin_count = opamp_pin_count,
    .create = opamp_create,
};

const PartKindOps OPAMP_KIND = {
    PART_OPAMP, "opamp", &OPAMP_MODEL, DEVICE_KEYS, PART_KEYS,
    opamp_load_device, opamp_bind_pinout, opamp_load_part, NULL, opamp_pin_analog
};
