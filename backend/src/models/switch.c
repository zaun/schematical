/*
 * switch: DIP switches (ported from backend/switch.c). The switch drives
 * nothing. Each position is a contact between two pins; while it is ON the
 * simulator resolves the nets on both pins as one (sim_set_contact), so a
 * value passes both ways. Every position starts OFF; the set_switch hook
 * turns one on or off, before or during a run.
 */

#include <stdlib.h>
#include <string.h>

#include "kind.h"

static const char *const DEVICE_KEYS[] = {"actuator", "contacts", NULL};

typedef struct {
    PartSwitch info;
    const char *(*contacts)[2];
} SwitchDevice;

typedef struct {
    size_t positions;
    size_t (*pins)[2];   /* pin indices of each position's contact */
} SwitchPins;

static void *switch_load_device(KindContext *ctx, const cJSON *json) {
    SwitchDevice *device = kind_alloc(ctx, sizeof(*device));
    const cJSON *contacts = kind_member(ctx, json, "contacts", 1);
    const cJSON *item;
    size_t n = 0;

    if (device == NULL || contacts == NULL ||
        kind_string(ctx, json, "actuator", &device->info.actuator) != 0) {
        return NULL;
    }
    if (!cJSON_IsArray(contacts) || cJSON_GetArraySize(contacts) == 0) {
        kind_error(ctx, "\"contacts\" must be a non-empty array of [pin, pin]");
        return NULL;
    }
    device->contacts = kind_alloc(ctx, (size_t)cJSON_GetArraySize(contacts) * sizeof(*device->contacts));
    if (device->contacts == NULL) {
        return NULL;
    }
    cJSON_ArrayForEach(item, contacts) {
        const cJSON *a = cJSON_GetArrayItem(item, 0);
        const cJSON *b = cJSON_GetArrayItem(item, 1);

        kind_push(ctx, "contacts[%zu]", n);
        if (!cJSON_IsArray(item) || cJSON_GetArraySize(item) != 2 || !cJSON_IsString(a) ||
            !cJSON_IsString(b)) {
            kind_error(ctx, "a contact is [pin name, pin name]");
            return NULL;
        }
        if (strcmp(a->valuestring, b->valuestring) == 0) {
            kind_error(ctx, "a contact joins two different pins, not \"%s\" twice", a->valuestring);
            return NULL;
        }
        device->contacts[n][0] = kind_strdup(ctx, a->valuestring);
        device->contacts[n][1] = kind_strdup(ctx, b->valuestring);
        kind_pop(ctx);
        n++;
    }
    device->info.positions = (unsigned)n;
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
    bound->pins = kind_alloc(ctx, bound->positions * sizeof(*bound->pins));
    if (bound->pins == NULL) {
        return NULL;
    }
    for (i = 0; i < bound->positions; i++) {
        kind_push(ctx, "contacts[%zu]", i);
        if (kind_pin(ctx, pins, pin_count, device->contacts[i][0], &bound->pins[i][0]) != 0 ||
            kind_pin(ctx, pins, pin_count, device->contacts[i][1], &bound->pins[i][1]) != 0) {
            return NULL;
        }
        kind_pop(ctx);
    }
    return bound;
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

    if (position < 1 || position > pins->positions) {
        return 0;
    }
    sim_set_contact(sim, instance, pins->pins[position - 1][0], pins->pins[position - 1][1], on);
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
