/*
 * diode (plan-29-H): rectifier, fast, switching, Schottky, Zener and bridge
 * parts. Passive: a diode drives nothing. Each (anode, cathode) pair of the
 * device is a one-way link in the engine (the `diodes` hook, sim.h): a 1
 * passes from anode to cathode and a 0 from cathode to anode. Pins that
 * carry the same name (a tab and a lead) are tied by links both ways.
 */

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "kind.h"
#include "model_pins.h"

static const char *const DEVICE_KEYS[] = {"type", "diodes", NULL};
static const char *const PART_KEYS[] = {"vr", "io", "vz", "pd", NULL};

static const char *const TYPES[] = {
    "rectifier", "fast", "switching", "schottky", "zener", "bridge", NULL
};

typedef struct {
    PartDiode info;
    const char *(*pairs)[2];  /* anode name, cathode name */
} DiodeDevice;

/* A pinout's links, as indices into Part.pins. */
typedef struct {
    size_t count;
    size_t *anode;
    size_t *cathode;
} DiodeLinks;

static void *diode_load_device(KindContext *ctx, const cJSON *json) {
    DiodeDevice *device = kind_alloc(ctx, sizeof(*device));
    const cJSON *pairs;
    const cJSON *item;
    size_t count = 0;
    size_t i;

    if (device == NULL || kind_string(ctx, json, "type", &device->info.type) != 0) {
        return NULL;
    }
    for (i = 0; TYPES[i] != NULL && strcmp(TYPES[i], device->info.type) != 0; i++) {
    }
    if (TYPES[i] == NULL) {
        kind_error(ctx, "unknown diode type \"%s\"", device->info.type);
        return NULL;
    }
    device->info.type = TYPES[i];
    pairs = kind_member(ctx, json, "diodes", 1);
    if (pairs == NULL) {
        return NULL;
    }
    if (!cJSON_IsArray(pairs) || cJSON_GetArraySize(pairs) == 0) {
        kind_error(ctx, "\"diodes\" must be a non-empty array of {\"anode\", \"cathode\"}");
        return NULL;
    }
    device->pairs = kind_alloc(ctx, (size_t)cJSON_GetArraySize(pairs) * sizeof(*device->pairs));
    if (device->pairs == NULL) {
        return NULL;
    }
    cJSON_ArrayForEach(item, pairs) {
        static const char *const PAIR_KEYS[] = {"anode", "cathode", NULL};

        kind_push(ctx, "diodes[%zu]", count);
        if (!cJSON_IsObject(item) || kind_check_keys(ctx, item, PAIR_KEYS, NULL) != 0 ||
            kind_string(ctx, item, "anode", &device->pairs[count][0]) != 0 ||
            kind_string(ctx, item, "cathode", &device->pairs[count][1]) != 0) {
            if (!kind_failed(ctx)) {
                kind_error(ctx, "a diode is {\"anode\": <pin name>, \"cathode\": <pin name>}");
            }
            return NULL;
        }
        if (strcmp(device->pairs[count][0], device->pairs[count][1]) == 0) {
            kind_error(ctx, "a diode's anode and cathode must be different pins");
            return NULL;
        }
        kind_pop(ctx);
        count++;
    }
    device->info.diode_count = count;
    return device;
}

static int add_link(KindContext *ctx, DiodeLinks *links, size_t cap, size_t a, size_t c) {
    if (links->count == cap) {
        return kind_error(ctx, "too many diode links");
    }
    links->anode[links->count] = a;
    links->cathode[links->count] = c;
    links->count++;
    return 0;
}

static void *diode_bind_pinout(KindContext *ctx, const void *raw, const PartPin *pins,
                               size_t pin_count) {
    const DiodeDevice *device = raw;
    DiodeLinks *links = kind_alloc(ctx, sizeof(*links));
    size_t *anodes = kind_alloc(ctx, (pin_count + 1) * sizeof(*anodes));
    size_t *cathodes = kind_alloc(ctx, (pin_count + 1) * sizeof(*cathodes));
    unsigned char *used = kind_alloc(ctx, pin_count + 1);
    size_t cap = device->info.diode_count * pin_count * pin_count + pin_count;
    size_t i;
    size_t k;

    if (links == NULL || anodes == NULL || cathodes == NULL || used == NULL) {
        return NULL;
    }
    for (i = 0; i < pin_count; i++) {
        PinRole role = pins[i].role;

        if (role != PIN_INOUT && role != PIN_NC && role != PIN_ABSENT) {
            kind_error(ctx, "diode pin \"%s\" must be inout", pins[i].name);
            return NULL;
        }
    }
    links->anode = kind_alloc(ctx, cap * sizeof(*links->anode));
    links->cathode = kind_alloc(ctx, cap * sizeof(*links->cathode));
    if (links->anode == NULL || links->cathode == NULL) {
        return NULL;
    }
    for (k = 0; k < device->info.diode_count; k++) {
        size_t na = kind_pins(ctx, pins, pin_count, device->pairs[k][0], anodes, pin_count);
        size_t nc;
        size_t a;
        size_t c;

        if (na == 0) {
            return NULL;
        }
        nc = kind_pins(ctx, pins, pin_count, device->pairs[k][1], cathodes, pin_count);
        if (nc == 0) {
            return NULL;
        }
        for (a = 0; a < na; a++) {
            used[anodes[a]] = 1;
            for (c = 0; c < nc; c++) {
                used[cathodes[c]] = 1;
                if (add_link(ctx, links, cap, anodes[a], cathodes[c]) != 0) {
                    return NULL;
                }
            }
        }
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
                if (add_link(ctx, links, cap, i, j) != 0 || add_link(ctx, links, cap, j, i) != 0) {
                    return NULL;
                }
            }
        }
    }
    for (i = 0; i < pin_count; i++) {
        if (pins[i].role == PIN_INOUT && !used[i]) {
            kind_error(ctx, "pin \"%s\" is in no diode", pins[i].name);
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

static int diode_load_part(KindContext *ctx, const void *raw, const void *bound,
                           const cJSON *json, Part *out) {
    const DiodeDevice *device = raw;
    PartDiode *info = kind_alloc(ctx, sizeof(*info));
    int zener = strcmp(device->info.type, "zener") == 0;
    double vr = 0, io = 0, vz = 0, pd = 0;
    int has_vr, has_io, has_vz, has_pd;

    (void)bound;
    if (info == NULL ||
        optional_number(ctx, json, "vr", 3000, &vr, &has_vr) != 0 ||
        optional_number(ctx, json, "io", 1000, &io, &has_io) != 0 ||
        optional_number(ctx, json, "vz", 1000, &vz, &has_vz) != 0 ||
        optional_number(ctx, json, "pd", 100, &pd, &has_pd) != 0) {
        return -1;
    }
    if (zener) {
        if (has_vr || has_io) {
            return kind_error(ctx, "\"%s\" is not a zener key", has_vr ? "vr" : "io");
        }
        if (!has_vz || !has_pd) {
            return kind_error(ctx, "missing \"%s\"", !has_vz ? "vz" : "pd");
        }
    } else {
        if (has_vz || has_pd) {
            return kind_error(ctx, "\"%s\" is only for a zener", has_vz ? "vz" : "pd");
        }
        if (!has_vr || !has_io) {
            return kind_error(ctx, "missing \"%s\"", !has_vr ? "vr" : "io");
        }
    }
    *info = device->info;
    info->vr_mv = (uint32_t)llround(vr * 1000);
    info->io_ma = (uint32_t)llround(io * 1000);
    info->vz_mv = (uint32_t)llround(vz * 1000);
    info->pd_mw = (uint32_t)llround(pd * 1000);
    out->diode = info;
    return 0;
}

int diode_pair(const Part *part, size_t k, size_t *anode, size_t *cathode) {
    const DiodeLinks *links;

    if (part == NULL || part->kind != PART_DIODE) {
        return 0;
    }
    links = part_internal(part)->bound;
    if (k >= links->count) {
        return 0;
    }
    *anode = links->anode[k];
    *cathode = links->cathode[k];
    return 1;
}

/* ---- Model --------------------------------------------------------------- */

static size_t diode_pin_count(const void *config) {
    return ((const Part *)config)->pin_count;
}

/* The state is the part's links (library memory, alive while the part is). */
static void *diode_create(const void *config) {
    return (void *)part_internal(config)->bound;
}

static size_t diode_links(const void *state, size_t *anode, size_t *cathode, size_t cap) {
    const DiodeLinks *links = state;
    size_t i;

    for (i = 0; i < links->count && i < cap; i++) {
        anode[i] = links->anode[i];
        cathode[i] = links->cathode[i];
    }
    return links->count;
}

static const SimComponentType DIODE_MODEL = {
    .name = "diode",
    .pin_count = diode_pin_count,
    .create = diode_create,
    .diodes = diode_links,
};

const PartKindOps DIODE_KIND = {
    PART_DIODE, "diode", &DIODE_MODEL, DEVICE_KEYS, PART_KEYS,
    diode_load_device, diode_bind_pinout, diode_load_part
};
