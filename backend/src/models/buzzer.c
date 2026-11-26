/*
 * buzzer (plan-47-J): board-mount buzzers, with a built-in driver
 * ("indicator") and without one ("transducer"), piezo and magnetic. A
 * two-terminal sounder: it is *on* while its positive pin reads 1 and its
 * negative pin reads 0, tracked and reported through the lights hooks exactly
 * as an LED is (lights.c), so the app can draw it and play its tone. It
 * drives nothing and has no timer. For a transducer driven by a square wave
 * the light's on-time is the wave's high time; the app derives the tone from
 * the net's change count. The facts (frequency, voltage range, current, sound
 * pressure) are data for the app.
 */

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "kind.h"
#include "lights.h"
#include "logic_alloc.h"

static const char *const DEVICE_KEYS[] = {"driver", "technology", "positive", "negative", NULL};
static const char *const PART_KEYS[] = {"hz", "volts", "voltsMin", "voltsMax", "ma", "db", NULL};
static const char *const DRIVERS[] = {"indicator", "transducer", NULL};
static const char *const TECHNOLOGIES[] = {"piezo", "magnetic", NULL};

typedef struct {
    const char *driver, *technology;
    const char *positive, *negative;
} BuzzerDevice;

/* The pins' indices into Part.pins. */
typedef struct {
    size_t positive, negative;
} BuzzerPins;

static const char *pick(const char *const *list, const char *text) {
    size_t i;

    for (i = 0; list[i] != NULL; i++) {
        if (strcmp(list[i], text) == 0) {
            return list[i];
        }
    }
    return NULL;
}

static void *buzzer_load_device(KindContext *ctx, const cJSON *json) {
    BuzzerDevice *device = kind_alloc(ctx, sizeof(*device));
    const char *text;

    if (device == NULL || kind_string(ctx, json, "driver", &text) != 0) {
        return NULL;
    }
    device->driver = pick(DRIVERS, text);
    if (device->driver == NULL) {
        kind_error(ctx, "unknown buzzer driver \"%s\"", text);
        return NULL;
    }
    if (kind_string(ctx, json, "technology", &text) != 0) {
        return NULL;
    }
    device->technology = pick(TECHNOLOGIES, text);
    if (device->technology == NULL) {
        kind_error(ctx, "unknown buzzer technology \"%s\"", text);
        return NULL;
    }
    if (kind_string(ctx, json, "positive", &device->positive) != 0 ||
        kind_string(ctx, json, "negative", &device->negative) != 0) {
        return NULL;
    }
    return device;
}

static void *buzzer_bind_pinout(KindContext *ctx, const void *raw, const PartPin *pins, size_t pin_count) {
    const BuzzerDevice *device = raw;
    BuzzerPins *bound = kind_alloc(ctx, sizeof(*bound));
    size_t i;

    if (bound == NULL || kind_pin(ctx, pins, pin_count, device->positive, &bound->positive) != 0 ||
        kind_pin(ctx, pins, pin_count, device->negative, &bound->negative) != 0) {
        return NULL;
    }
    if (bound->positive == bound->negative) {
        kind_error(ctx, "\"positive\" and \"negative\" must name different pins");
        return NULL;
    }
    for (i = 0; i < pin_count; i++) {
        if (i == bound->positive || i == bound->negative) {
            if (pins[i].role != PIN_INPUT) {
                kind_error(ctx, "buzzer pin \"%s\" must be input", pins[i].name);
                return NULL;
            }
        } else if (pins[i].role != PIN_NC) {
            kind_error(ctx, "buzzer pin \"%s\" must be nc", pins[i].name);
            return NULL;
        }
    }
    return bound;
}

/* An optional number above 0 and at most max, scaled by `unit` and rounded (at least 1); 0 when absent. */
static int scaled(KindContext *ctx, const cJSON *json, const char *key, double max, double unit,
                  uint32_t *out) {
    const cJSON *item = kind_member(ctx, json, key, 0);

    *out = 0;
    if (item == NULL) {
        return 0;
    }
    if (!cJSON_IsNumber(item) || !(item->valuedouble > 0) || item->valuedouble > max) {
        return kind_error(ctx, "\"%s\" must be above 0 and at most %g", key, max);
    }
    *out = (uint32_t)llround(item->valuedouble * unit);
    if (*out == 0) {
        *out = 1;
    }
    return 0;
}

static int buzzer_load_part(KindContext *ctx, const void *raw, const void *bound, const cJSON *json,
                            Part *out) {
    const BuzzerDevice *device = raw;
    PartBuzzer *info = kind_alloc(ctx, sizeof(*info));
    uint64_t hz;
    uint32_t ma = 0;
    uint32_t db = 0;

    (void)bound;
    if (info == NULL) {
        return -1;
    }
    info->driver = device->driver;
    info->technology = device->technology;
    if (kind_uint(ctx, json, "hz", 100000, &hz) != 0) {
        return -1;
    }
    if (hz < 1) {
        return kind_error(ctx, "\"hz\" must be at least 1");
    }
    info->hz = (uint32_t)hz;
    if (scaled(ctx, json, "volts", 1000, 1000.0, &info->rated_mv) != 0 ||
        scaled(ctx, json, "voltsMin", 1000, 1000.0, &info->min_mv) != 0 ||
        scaled(ctx, json, "voltsMax", 1000, 1000.0, &info->max_mv) != 0 ||
        scaled(ctx, json, "ma", 10000, 1000.0, &ma) != 0 || scaled(ctx, json, "db", 200, 100.0, &db) != 0) {
        return -1;
    }
    if (info->min_mv > 0 && info->max_mv > 0 && info->min_mv > info->max_mv) {
        return kind_error(ctx, "\"voltsMin\" must be at most \"voltsMax\"");
    }
    info->ua = ma;           /* milliamperes * 1000 = microamperes */
    info->spl_cdb = db;
    out->buzzer = info;
    return 0;
}

/* ---- Model --------------------------------------------------------------- */

typedef struct {
    BuzzerPins pins;
    Light light;
} BuzzerState;

static size_t buzzer_pin_count(const void *config) {
    return ((const Part *)config)->pin_count;
}

static void *buzzer_create(const void *config) {
    BuzzerState *state = logic_calloc(1, sizeof(*state));

    if (state != NULL) {
        state->pins = *(const BuzzerPins *)part_internal(config)->bound;
    }
    return state;
}

static void buzzer_destroy(void *state) {
    logic_free(state);
}

static void buzzer_update(Sim *sim, SimInstance *instance, BuzzerState *state) {
    int on = sim_read_pin(sim, instance, state->pins.positive) == LOGIC_1 &&
             sim_read_pin(sim, instance, state->pins.negative) == LOGIC_0;

    lights_set(&state->light, 0, on, sim_now(sim));
}

static void buzzer_start(Sim *sim, SimInstance *instance, void *state) {
    buzzer_update(sim, instance, state);
}

static void buzzer_pin_changed(Sim *sim, SimInstance *instance, void *state, size_t pin,
                               LogicValue old_value, LogicValue new_value) {
    (void)pin;
    (void)old_value;
    (void)new_value;
    buzzer_update(sim, instance, state);
}

static size_t buzzer_lights(const Sim *sim, const SimInstance *instance, const void *state, SimTime since,
                            unsigned char *out, size_t cap) {
    (void)sim;
    (void)instance;
    return lights_read(&((const BuzzerState *)state)->light, 1, since, out, cap);
}

static size_t buzzer_light_time(const Sim *sim, const SimInstance *instance, const void *state,
                                uint64_t *out, size_t cap) {
    (void)instance;
    return lights_time(&((const BuzzerState *)state)->light, 1, sim_now(sim), out, cap);
}

static const SimComponentType BUZZER_MODEL = {
    .name = "buzzer",
    .pin_count = buzzer_pin_count,
    .create = buzzer_create,
    .destroy = buzzer_destroy,
    .start = buzzer_start,
    .pin_changed = buzzer_pin_changed,
    .lights = buzzer_lights,
    .light_time = buzzer_light_time,
};

const PartKindOps BUZZER_KIND = {
    PART_BUZZER, "buzzer", &BUZZER_MODEL, DEVICE_KEYS, PART_KEYS,
    buzzer_load_device, buzzer_bind_pinout, buzzer_load_part
};
