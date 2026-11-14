/*
 * resistor (plan-34-E): fixed resistors and resistor networks / arrays.
 * Passive: a resistor drives nothing. Each (a, b) pair of the device is a
 * two-way weak link in the engine (the `resistors` hook, sim.h): each end
 * sees the other end's level as a weak drive, so a resistor to VCC is a
 * pull-up and a real driver overrides it. Ohms have no effect on logic.
 * Pins that carry the same name (an array's two common pins) are one node:
 * they are tied by diode links both ways (the `diodes` hook), as diode.c
 * ties a tab to its lead.
 */

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "kind.h"
#include "model_pins.h"

static const char *const DEVICE_KEYS[] = {"resistors", NULL};
static const char *const PART_KEYS[] = {"ohms", "tolerance", "power", NULL};

typedef struct {
    size_t count;
    const char *(*pairs)[2];  /* a name, b name */
} ResistorDevice;

/* A pinout's links, as indices into Part.pins: the resistor links, and the
   ties between same-named pins (each tie twice, one per direction). */
typedef struct {
    size_t count;
    size_t *a;
    size_t *b;
    size_t tie_count;
    size_t *tie_from;
    size_t *tie_to;
} ResistorLinks;

static void *resistor_load_device(KindContext *ctx, const cJSON *json) {
    ResistorDevice *device = kind_alloc(ctx, sizeof(*device));
    const cJSON *pairs;
    const cJSON *item;
    size_t count = 0;

    if (device == NULL) {
        return NULL;
    }
    pairs = kind_member(ctx, json, "resistors", 1);
    if (pairs == NULL) {
        return NULL;
    }
    if (!cJSON_IsArray(pairs) || cJSON_GetArraySize(pairs) == 0) {
        kind_error(ctx, "\"resistors\" must be a non-empty array of {\"a\", \"b\"}");
        return NULL;
    }
    device->pairs = kind_alloc(ctx, (size_t)cJSON_GetArraySize(pairs) * sizeof(*device->pairs));
    if (device->pairs == NULL) {
        return NULL;
    }
    cJSON_ArrayForEach(item, pairs) {
        static const char *const PAIR_KEYS[] = {"a", "b", NULL};

        kind_push(ctx, "resistors[%zu]", count);
        if (!cJSON_IsObject(item) || kind_check_keys(ctx, item, PAIR_KEYS, NULL) != 0 ||
            kind_string(ctx, item, "a", &device->pairs[count][0]) != 0 ||
            kind_string(ctx, item, "b", &device->pairs[count][1]) != 0) {
            if (!kind_failed(ctx)) {
                kind_error(ctx, "a resistor is {\"a\": <pin name>, \"b\": <pin name>}");
            }
            return NULL;
        }
        if (strcmp(device->pairs[count][0], device->pairs[count][1]) == 0) {
            kind_error(ctx, "a resistor's a and b must be different pins");
            return NULL;
        }
        kind_pop(ctx);
        count++;
    }
    device->count = count;
    return device;
}

static void *resistor_bind_pinout(KindContext *ctx, const void *raw, const PartPin *pins,
                                  size_t pin_count) {
    const ResistorDevice *device = raw;
    ResistorLinks *links = kind_alloc(ctx, sizeof(*links));
    size_t *as = kind_alloc(ctx, (pin_count + 1) * sizeof(*as));
    size_t *bs = kind_alloc(ctx, (pin_count + 1) * sizeof(*bs));
    unsigned char *used = kind_alloc(ctx, pin_count + 1);
    size_t cap = device->count * pin_count * pin_count + 1;
    size_t tie_cap = 2 * pin_count + 1;
    size_t i;
    size_t k;

    if (links == NULL || as == NULL || bs == NULL || used == NULL) {
        return NULL;
    }
    for (i = 0; i < pin_count; i++) {
        PinRole role = pins[i].role;

        if (role != PIN_INOUT && role != PIN_NC && role != PIN_ABSENT) {
            kind_error(ctx, "resistor pin \"%s\" must be inout", pins[i].name);
            return NULL;
        }
    }
    links->a = kind_alloc(ctx, cap * sizeof(*links->a));
    links->b = kind_alloc(ctx, cap * sizeof(*links->b));
    links->tie_from = kind_alloc(ctx, tie_cap * sizeof(*links->tie_from));
    links->tie_to = kind_alloc(ctx, tie_cap * sizeof(*links->tie_to));
    if (links->a == NULL || links->b == NULL || links->tie_from == NULL || links->tie_to == NULL) {
        return NULL;
    }
    for (k = 0; k < device->count; k++) {
        size_t na = kind_pins(ctx, pins, pin_count, device->pairs[k][0], as, pin_count);
        size_t nb;
        size_t x;
        size_t y;

        if (na == 0) {
            return NULL;
        }
        nb = kind_pins(ctx, pins, pin_count, device->pairs[k][1], bs, pin_count);
        if (nb == 0) {
            return NULL;
        }
        /* A same-named pin is tied to its first pin, so one link from each
           name's first pin is enough. */
        used[as[0]] = 1;
        used[bs[0]] = 1;
        for (x = 1; x < na; x++) {
            used[as[x]] = 1;
        }
        for (y = 1; y < nb; y++) {
            used[bs[y]] = 1;
        }
        links->a[links->count] = as[0];
        links->b[links->count] = bs[0];
        links->count++;
    }
    /* Same-named pins: tied both ways, first pin to each other one. */
    for (i = 0; i < pin_count; i++) {
        size_t j;

        if (pins[i].role != PIN_INOUT) {
            continue;
        }
        for (j = 0; j < i && strcmp(pins[j].name, pins[i].name) != 0; j++) {
        }
        if (j < i) {
            continue;  /* not the first pin of its name */
        }
        for (j = i + 1; j < pin_count; j++) {
            if (pins[j].role == PIN_INOUT && strcmp(pins[j].name, pins[i].name) == 0) {
                if (links->tie_count + 2 > tie_cap) {
                    kind_error(ctx, "too many same-named resistor pins");
                    return NULL;
                }
                links->tie_from[links->tie_count] = i;
                links->tie_to[links->tie_count++] = j;
                links->tie_from[links->tie_count] = j;
                links->tie_to[links->tie_count++] = i;
            }
        }
    }
    for (i = 0; i < pin_count; i++) {
        if (pins[i].role == PIN_INOUT && !used[i]) {
            kind_error(ctx, "pin \"%s\" is in no resistor", pins[i].name);
            return NULL;
        }
    }
    return links;
}

/* An optional part number in (0, max]; *present says whether it was given. */
static int optional_number(KindContext *ctx, const cJSON *json, const char *key, double max,
                           double *out, int *present) {
    const cJSON *item = kind_member(ctx, json, key, 0);

    *present = item != NULL;
    if (item == NULL) {
        return 0;
    }
    if (!cJSON_IsNumber(item) || !(item->valuedouble > 0) || item->valuedouble > max) {
        return kind_error(ctx, "\"%s\" must be above 0 and at most %g", key, max);
    }
    *out = item->valuedouble;
    return 0;
}

static int resistor_load_part(KindContext *ctx, const void *raw, const void *bound,
                              const cJSON *json, Part *out) {
    const ResistorDevice *device = raw;
    PartResistor *info = kind_alloc(ctx, sizeof(*info));
    const cJSON *ohms;
    double tolerance = 0, power = 0;
    int has_tolerance, has_power;

    (void)bound;
    if (info == NULL) {
        return -1;
    }
    ohms = kind_member(ctx, json, "ohms", 1);
    if (ohms == NULL) {
        return -1;
    }
    if (!cJSON_IsNumber(ohms) || !(ohms->valuedouble >= 0) || ohms->valuedouble > 1e9) {
        return kind_error(ctx, "\"ohms\" must be at least 0 and at most 1e9");
    }
    if (optional_number(ctx, json, "tolerance", 20, &tolerance, &has_tolerance) != 0 ||
        optional_number(ctx, json, "power", 100, &power, &has_power) != 0) {
        return -1;
    }
    info->resistor_count = device->count;
    info->milliohms = (uint64_t)llround(ohms->valuedouble * 1000);
    info->tolerance_ppm = has_tolerance ? (uint32_t)llround(tolerance * 10000) : 0;
    info->power_mw = has_power ? (uint32_t)llround(power * 1000) : 0;
    out->resistor = info;
    return 0;
}

int resistor_pair(const Part *part, size_t k, size_t *a, size_t *b) {
    const ResistorLinks *links;

    if (part == NULL || part->kind != PART_RESISTOR) {
        return 0;
    }
    links = part_internal(part)->bound;
    if (k < links->count) {
        *a = links->a[k];
        *b = links->b[k];
        return 1;
    }
    k -= links->count;
    if (k < links->tie_count) {
        *a = links->tie_from[k];
        *b = links->tie_to[k];
        return 1;
    }
    return 0;
}

/* ---- Model --------------------------------------------------------------- */

static size_t resistor_pin_count(const void *config) {
    return ((const Part *)config)->pin_count;
}

/* The state is the part's links (library memory, alive while the part is). */
static void *resistor_create(const void *config) {
    return (void *)part_internal(config)->bound;
}

static size_t resistor_links(const void *state, size_t *a, size_t *b, size_t cap) {
    const ResistorLinks *links = state;
    size_t i;

    for (i = 0; i < links->count && i < cap; i++) {
        a[i] = links->a[i];
        b[i] = links->b[i];
    }
    return links->count;
}

static size_t resistor_ties(const void *state, size_t *from, size_t *to, size_t cap) {
    const ResistorLinks *links = state;
    size_t i;

    for (i = 0; i < links->tie_count && i < cap; i++) {
        from[i] = links->tie_from[i];
        to[i] = links->tie_to[i];
    }
    return links->tie_count;
}

static const SimComponentType RESISTOR_MODEL = {
    .name = "resistor",
    .pin_count = resistor_pin_count,
    .create = resistor_create,
    .diodes = resistor_ties,
    .resistors = resistor_links,
};

const PartKindOps RESISTOR_KIND = {
    PART_RESISTOR, "resistor", &RESISTOR_MODEL, DEVICE_KEYS, PART_KEYS,
    resistor_load_device, resistor_bind_pinout, resistor_load_part
};
