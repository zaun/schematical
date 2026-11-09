/*
 * switch: DIP switches (ported from backend/switch.c). The switch drives
 * nothing. Each position is a contact between two pins; while it is ON the
 * simulator resolves the nets on both pins as one (sim_set_contact), so a
 * value passes both ways. Every position starts OFF; the set_switch hook
 * turns one on or off, before or during a run.
 *
 * Tactile switches (plan-27-D) use the same kind: "momentary" (data for the
 * app), "normallyClosed" (the circuit records the positions closed when the
 * part is added) and "joined" pin pairs, which are contacts numbered after
 * the positions that are always closed and that set_switch cannot address.
 * "bodySize" (optional) is the nominal body [length, width] in mm, data for
 * the app's drawer.
 */

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "kind.h"
#include "model_pins.h"

static const char *const DEVICE_KEYS[] = {"actuator", "contacts", "ratedVolts", "momentary",
                                           "normallyClosed", "joined", "bodySize", NULL};

typedef struct {
    PartSwitch info;
    const char *(*contacts)[2];  /* positions, then joined pairs */
} SwitchDevice;

typedef struct {
    size_t positions;
    size_t joined;
    size_t (*pins)[2];   /* pin indices of each contact: positions, then joined pairs */
} SwitchPins;

/* Reads an optional bool key; 0 when absent. */
static int optional_bool(KindContext *ctx, const cJSON *json, const char *key, unsigned *out) {
    int value = 0;

    if (cJSON_GetObjectItemCaseSensitive(json, key) != NULL &&
        kind_bool(ctx, json, key, &value) != 0) {
        return -1;
    }
    *out = (unsigned)value;
    return 0;
}

/* Reads one [pin, pin] pair; what names the pair in messages ("contact", "joined pair"). */
static int read_pair(KindContext *ctx, const cJSON *item, const char *what, const char **a_out,
                     const char **b_out) {
    const cJSON *a = cJSON_GetArrayItem(item, 0);
    const cJSON *b = cJSON_GetArrayItem(item, 1);

    if (!cJSON_IsArray(item) || cJSON_GetArraySize(item) != 2 || !cJSON_IsString(a) ||
        !cJSON_IsString(b)) {
        return kind_error(ctx, "a %s is [pin name, pin name]", what);
    }
    if (strcmp(a->valuestring, b->valuestring) == 0) {
        return kind_error(ctx, "a %s joins two different pins, not \"%s\" twice", what,
                          a->valuestring);
    }
    *a_out = kind_strdup(ctx, a->valuestring);
    *b_out = kind_strdup(ctx, b->valuestring);
    return *a_out != NULL && *b_out != NULL ? 0 : -1;
}

/* Reads the optional "bodySize": [length, width], each above 0 and at most
   100 mm, length >= width. */
static int read_body_size(KindContext *ctx, const cJSON *json, uint32_t out[2]) {
    const cJSON *size = cJSON_GetObjectItemCaseSensitive(json, "bodySize");
    const cJSON *length = cJSON_GetArrayItem(size, 0);
    const cJSON *width = cJSON_GetArrayItem(size, 1);

    out[0] = out[1] = 0;
    if (size == NULL) {
        return 0;
    }
    if (!cJSON_IsArray(size) || cJSON_GetArraySize(size) != 2 || !cJSON_IsNumber(length) ||
        !cJSON_IsNumber(width) || length->valuedouble <= 0 || width->valuedouble <= 0 ||
        length->valuedouble > 100 || width->valuedouble > 100) {
        return kind_error(ctx, "\"bodySize\" must be [length, width] in mm, each above 0 and at most 100");
    }
    if (length->valuedouble < width->valuedouble) {
        return kind_error(ctx, "\"bodySize\" gives the length (the larger side) first");
    }
    out[0] = (uint32_t)llround(length->valuedouble * 1000);
    out[1] = (uint32_t)llround(width->valuedouble * 1000);
    return 0;
}

static void *switch_load_device(KindContext *ctx, const cJSON *json) {
    SwitchDevice *device = kind_alloc(ctx, sizeof(*device));
    const cJSON *contacts = kind_member(ctx, json, "contacts", 1);
    const cJSON *joined = cJSON_GetObjectItemCaseSensitive(json, "joined");
    const cJSON *item;
    size_t n = 0;
    size_t joined_count = 0;

    if (device == NULL || contacts == NULL ||
        kind_string(ctx, json, "actuator", &device->info.actuator) != 0) {
        return NULL;
    }
    if (!cJSON_IsArray(contacts) || cJSON_GetArraySize(contacts) == 0) {
        kind_error(ctx, "\"contacts\" must be a non-empty array of [pin, pin]");
        return NULL;
    }
    if (joined != NULL) {
        if (!cJSON_IsArray(joined) || cJSON_GetArraySize(joined) == 0) {
            kind_error(ctx, "\"joined\" must be a non-empty array of [pin, pin]");
            return NULL;
        }
        joined_count = (size_t)cJSON_GetArraySize(joined);
    }
    device->contacts = kind_alloc(ctx, ((size_t)cJSON_GetArraySize(contacts) + joined_count) *
                                           sizeof(*device->contacts));
    if (device->contacts == NULL) {
        return NULL;
    }
    cJSON_ArrayForEach(item, contacts) {
        kind_push(ctx, "contacts[%zu]", n);
        if (read_pair(ctx, item, "contact", &device->contacts[n][0], &device->contacts[n][1]) != 0) {
            return NULL;
        }
        kind_pop(ctx);
        n++;
    }
    device->info.positions = (unsigned)n;
    if (joined != NULL) {
        size_t j = 0;
        cJSON_ArrayForEach(item, joined) {
            kind_push(ctx, "joined[%zu]", j);
            if (read_pair(ctx, item, "joined pair", &device->contacts[n + j][0],
                          &device->contacts[n + j][1]) != 0) {
                return NULL;
            }
            kind_pop(ctx);
            j++;
        }
    }
    device->info.joined = (unsigned)joined_count;
    if (optional_bool(ctx, json, "momentary", &device->info.momentary) != 0 ||
        optional_bool(ctx, json, "normallyClosed", &device->info.normally_closed) != 0 ||
        read_body_size(ctx, json, device->info.body_um) != 0) {
        return NULL;
    }
    if (device->info.normally_closed && !device->info.momentary) {
        kind_error(ctx, "\"normallyClosed\" needs \"momentary\"");
        return NULL;
    }
    /* ratedVolts: the contacts' switching rating. */
    if (kind_volts(ctx, json, "ratedVolts", 1000, &device->info.rated_mv) != 0) {
        return NULL;
    }
    if (device->info.rated_mv == 0) {
        kind_error(ctx, "\"ratedVolts\" must be above 0");
        return NULL;
    }
    return device;
}

static void *switch_bind_pinout(KindContext *ctx, const void *raw, const PartPin *pins,
                                size_t pin_count) {
    const SwitchDevice *device = raw;
    SwitchPins *bound = kind_alloc(ctx, sizeof(*bound));
    size_t i;

    if (bound == NULL) {
        return NULL;
    }
    bound->positions = device->info.positions;
    bound->joined = device->info.joined;
    bound->pins = kind_alloc(ctx, (bound->positions + bound->joined) * sizeof(*bound->pins));
    if (bound->pins == NULL) {
        return NULL;
    }
    for (i = 0; i < bound->positions + bound->joined; i++) {
        if (i < bound->positions) {
            kind_push(ctx, "contacts[%zu]", i);
        } else {
            kind_push(ctx, "joined[%zu]", i - bound->positions);
        }
        if (kind_pin(ctx, pins, pin_count, device->contacts[i][0], &bound->pins[i][0]) != 0 ||
            kind_pin(ctx, pins, pin_count, device->contacts[i][1], &bound->pins[i][1]) != 0) {
            return NULL;
        }
        kind_pop(ctx);
    }
    return bound;
}

int switch_contact_pins(const Part *part, unsigned position, size_t *a, size_t *b) {
    const SwitchPins *pins;

    if (part == NULL || part->kind != PART_SWITCH) {
        return 0;
    }
    pins = part_internal(part)->bound;
    if (position < 1 || position > pins->positions + pins->joined) {
        return 0;
    }
    *a = pins->pins[position - 1][0];
    *b = pins->pins[position - 1][1];
    return 1;
}

static int switch_load_part(KindContext *ctx, const void *device, const void *bound,
                            const cJSON *json, Part *out) {
    (void)ctx;
    (void)bound;
    (void)json;
    out->dip_switch = &((const SwitchDevice *)device)->info;
    return 0;
}

/* ---- Model --------------------------------------------------------------- */

static size_t switch_pin_count(const void *config) {
    return ((const Part *)config)->pin_count;
}

static void *switch_create(const void *config) {
    return (void *)part_internal(config)->bound;
}

static int switch_set(Sim *sim, SimInstance *instance, void *state, size_t position, int on) {
    const SwitchPins *pins = state;

    size_t j;

    if (position < 1 || position > pins->positions) {
        return 0;
    }
    sim_set_contact(sim, instance, pins->pins[position - 1][0], pins->pins[position - 1][1], on);
    for (j = pins->positions; j < pins->positions + pins->joined; j++) {
        sim_set_contact(sim, instance, pins->pins[j][0], pins->pins[j][1], 1);
    }
    return 1;
}

static const SimComponentType SWITCH_MODEL = {
    "switch", switch_pin_count, switch_create, NULL, NULL, NULL, NULL,
    NULL, switch_set, NULL, NULL
};

const PartKindOps SWITCH_KIND = {
    PART_SWITCH, "switch", &SWITCH_MODEL, DEVICE_KEYS, NULL,
    switch_load_device, switch_bind_pinout, switch_load_part
};
