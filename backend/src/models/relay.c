/*
 * relay (plan-47-J): electromechanical, reed and solid-state relays. A relay
 * has coils (a solid-state relay's input LEDs) and poles of contacts. Each
 * coil entry {a, b} is an input pair; a coil is *driven* forward when a reads
 * 1 and b reads 0 and backward when a reads 0 and b reads 1 (a polarized coil
 * is driven forward only); X and Z on either pin is not driven. A channel (an
 * independent armature) is operated while its coil is driven; a latching relay
 * has one channel and keeps its position until it is driven the other way
 * (one coil: forward sets, backward resets; two coils: the first sets, the
 * second resets, both or neither change nothing). The armature moves 1 ms
 * (RELAY_DELAY_PS) after the coil last changed: every pole of the channel
 * opens its old contact and closes its new one in one event (sim_set_contact,
 * as a switch's and a transistor's channel). Each pole is a common pin with a
 * normally-open `no` and / or a normally-closed `nc` pin. A coil is not a
 * conductor: nothing passes between its pins. The facts (coil voltage and
 * resistance, contact rating, operate and release times, on-resistance) are
 * data for the app. The lights hooks report one light per channel: on while
 * the armature is operated.
 */

#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "kind.h"
#include "lights.h"
#include "model_pins.h"
#include "terminals.h"
#include "logic_alloc.h"

#define RELAY_DELAY_PS 1000000000ULL   /* 1 ms, every relay, both ways */
#define MAX_COILS 8
#define MAX_POLES 8
#define NONE ((size_t)-1)

static const char *const DEVICE_KEYS[] = {"type", "coils", "latching", "contacts", NULL};
static const char *const PART_KEYS[] = {"coilVolts", "coilOhms", "contactAmps", "contactVolts",
                                         "operateMs", "releaseMs", "onOhms", NULL};
static const char *const TYPES[] = {"electromechanical", "reed", "solid-state", NULL};
static const char *const COIL_KEYS[] = {"a", "b", "polarized", NULL};
static const char *const POLE_KEYS[] = {"common", "no", "nc", "coil", NULL};

typedef struct {
    const char *a, *b;
    int polarized;
} CoilEntry;

typedef struct {
    const char *common, *no, *nc;
    unsigned coil;               /* 1-based */
} PoleEntry;

typedef struct {
    PartRelay info;              /* the device's part: type, poles, coils, latching, channels */
    CoilEntry coil[MAX_COILS];
    PoleEntry pole[MAX_POLES];
} RelayDevice;

/* A pinout's pins, as indices into Part.pins. The coil pins read are the first pin of
   each coil name. A pole's contacts are one contact between every pin of its common
   name and every pin of its no (or nc) name, so any of them may be the wired one;
   the ties join the pins of one name with each other (terminals.h). */
typedef struct {
    size_t coil_a[MAX_COILS], coil_b[MAX_COILS];
    int polarized[MAX_COILS];
    ContactSet no[MAX_POLES], nc[MAX_POLES];   /* empty when the pole has no such contact */
    unsigned channel[MAX_POLES];               /* 0-based */
    ContactSet ties;
} RelayPins;

static int optional_bool(KindContext *ctx, const cJSON *json, const char *key, int *out) {
    *out = 0;
    if (cJSON_GetObjectItemCaseSensitive(json, key) == NULL) {
        return 0;
    }
    return kind_bool(ctx, json, key, out);
}

static void *relay_load_device(KindContext *ctx, const cJSON *json) {
    RelayDevice *device = kind_alloc(ctx, sizeof(*device));
    const cJSON *coils;
    const cJSON *contacts;
    const cJSON *item;
    const char *type;
    size_t i;
    size_t n;
    int latching;
    unsigned channel_has_pole[MAX_COILS] = {0};

    if (device == NULL || kind_string(ctx, json, "type", &type) != 0) {
        return NULL;
    }
    for (i = 0; TYPES[i] != NULL && strcmp(TYPES[i], type) != 0; i++) {
    }
    if (TYPES[i] == NULL) {
        kind_error(ctx, "unknown relay type \"%s\"", type);
        return NULL;
    }
    device->info.type = TYPES[i];
    coils = cJSON_GetObjectItemCaseSensitive(json, "coils");
    if (!cJSON_IsArray(coils) || cJSON_GetArraySize(coils) < 1 || cJSON_GetArraySize(coils) > MAX_COILS) {
        kind_error(ctx, "\"coils\" must have 1 to %d entries", MAX_COILS);
        return NULL;
    }
    device->info.coils = (unsigned)cJSON_GetArraySize(coils);
    if (optional_bool(ctx, json, "latching", &latching) != 0) {
        return NULL;
    }
    device->info.latching = latching;
    if (latching && i == 2) {
        kind_error(ctx, "a solid-state relay is not latching");
        return NULL;
    }
    if (latching && device->info.coils > 2) {
        kind_error(ctx, "a latching relay has 1 or 2 coils");
        return NULL;
    }
    device->info.channels = latching ? 1 : device->info.coils;
    n = 0;
    cJSON_ArrayForEach(item, coils) {
        kind_push(ctx, "coils[%zu]", n);
        if (!cJSON_IsObject(item) || kind_check_keys(ctx, item, COIL_KEYS, NULL) != 0 ||
            kind_string(ctx, item, "a", &device->coil[n].a) != 0 ||
            kind_string(ctx, item, "b", &device->coil[n].b) != 0 ||
            optional_bool(ctx, item, "polarized", &device->coil[n].polarized) != 0) {
            if (!kind_failed(ctx)) {
                kind_error(ctx, "a coil is {\"a\": <pin name>, \"b\": <pin name>}");
            }
            return NULL;
        }
        if (strcmp(device->coil[n].a, device->coil[n].b) == 0) {
            kind_error(ctx, "a coil's a and b must be different pins");
            return NULL;
        }
        if (i == 2) {
            device->coil[n].polarized = 1;      /* an input LED */
        }
        if (latching && device->info.coils == 1 && device->coil[n].polarized) {
            kind_error(ctx, "a single-coil latching relay is not \"polarized\"");
            return NULL;
        }
        kind_pop(ctx);
        n++;
    }
    contacts = cJSON_GetObjectItemCaseSensitive(json, "contacts");
    if (!cJSON_IsArray(contacts) || cJSON_GetArraySize(contacts) < 1 ||
        cJSON_GetArraySize(contacts) > MAX_POLES) {
        kind_error(ctx, "\"contacts\" must have 1 to %d entries", MAX_POLES);
        return NULL;
    }
    device->info.poles = (unsigned)cJSON_GetArraySize(contacts);
    n = 0;
    cJSON_ArrayForEach(item, contacts) {
        PoleEntry *pole = &device->pole[n];
        const cJSON *coil;

        kind_push(ctx, "contacts[%zu]", n);
        if (!cJSON_IsObject(item) || kind_check_keys(ctx, item, POLE_KEYS, NULL) != 0 ||
            kind_string(ctx, item, "common", &pole->common) != 0) {
            if (!kind_failed(ctx)) {
                kind_error(ctx, "a contact is {\"common\", \"no\", \"nc\"}");
            }
            return NULL;
        }
        if (cJSON_GetObjectItemCaseSensitive(item, "no") != NULL &&
            kind_string(ctx, item, "no", &pole->no) != 0) {
            return NULL;
        }
        if (cJSON_GetObjectItemCaseSensitive(item, "nc") != NULL &&
            kind_string(ctx, item, "nc", &pole->nc) != 0) {
            return NULL;
        }
        if (pole->no == NULL && pole->nc == NULL) {
            kind_error(ctx, "a contact needs \"no\" or \"nc\"");
            return NULL;
        }
        if ((pole->no != NULL && strcmp(pole->common, pole->no) == 0) ||
            (pole->nc != NULL && strcmp(pole->common, pole->nc) == 0) ||
            (pole->no != NULL && pole->nc != NULL && strcmp(pole->no, pole->nc) == 0)) {
            kind_error(ctx, "the names of a contact must be different pins");
            return NULL;
        }
        pole->coil = 1;
        coil = cJSON_GetObjectItemCaseSensitive(item, "coil");
        if (coil != NULL) {
            uint64_t value;

            if (latching) {
                kind_error(ctx, "\"coil\" is not for a latching relay");
                return NULL;
            }
            if (!cJSON_IsNumber(coil) || coil->valuedouble < 1 || coil->valuedouble > device->info.coils ||
                coil->valuedouble != floor(coil->valuedouble)) {
                kind_error(ctx, "\"coil\" must be 1 to %u", device->info.coils);
                return NULL;
            }
            value = (uint64_t)coil->valuedouble;
            pole->coil = (unsigned)value;
        }
        channel_has_pole[latching ? 0 : pole->coil - 1] = 1;
        kind_pop(ctx);
        n++;
    }
    for (i = 0; i < device->info.channels; i++) {
        if (!channel_has_pole[i]) {
            kind_error(ctx, "coil %zu has no contact", i + 1);
            return NULL;
        }
    }
    return device;
}

/* How many entries name pin `name`, and whether any of them is a contact. */
typedef struct {
    unsigned entries;
    unsigned contact_entries;
} Use;

static void count_uses(const RelayDevice *device, const PartPin *pins, size_t pin_count, Use *use) {
    size_t i;
    size_t k;

    for (i = 0; i < pin_count; i++) {
        for (k = 0; k < device->info.coils; k++) {
            if (strcmp(pins[i].name, device->coil[k].a) == 0 || strcmp(pins[i].name, device->coil[k].b) == 0) {
                use[i].entries++;
            }
        }
        for (k = 0; k < device->info.poles; k++) {
            const PoleEntry *pole = &device->pole[k];

            if (strcmp(pins[i].name, pole->common) == 0 ||
                (pole->no != NULL && strcmp(pins[i].name, pole->no) == 0) ||
                (pole->nc != NULL && strcmp(pins[i].name, pole->nc) == 0)) {
                use[i].entries++;
                use[i].contact_entries++;
            }
        }
    }
}

/* The first pin of a name; NONE for a NULL name; NONE with an error set when it names no pin. */
static int first_pin(KindContext *ctx, const PartPin *pins, size_t pin_count, const char *name,
                     size_t *tmp, size_t *out) {
    if (name == NULL) {
        *out = NONE;
        return 0;
    }
    if (kind_pins(ctx, pins, pin_count, name, tmp, pin_count) == 0) {
        return -1;
    }
    *out = tmp[0];
    return 0;
}

static void *relay_bind_pinout(KindContext *ctx, const void *raw, const PartPin *pins, size_t pin_count) {
    const RelayDevice *device = raw;
    RelayPins *bound = kind_alloc(ctx, sizeof(*bound));
    Use *use = kind_alloc(ctx, (pin_count + 1) * sizeof(*use));
    size_t *tmp = kind_alloc(ctx, (pin_count + 1) * sizeof(*tmp));
    size_t k;
    size_t i;

    if (bound == NULL || use == NULL || tmp == NULL) {
        return NULL;
    }
    if (contacts_new(ctx, &bound->ties, pin_count * pin_count) != 0) {
        return NULL;
    }
    count_uses(device, pins, pin_count, use);
    for (i = 0; i < pin_count; i++) {
        int coil_pin = 0;
        int contact_pin = use[i].contact_entries > 0;

        for (k = 0; k < device->info.coils; k++) {
            if (strcmp(pins[i].name, device->coil[k].a) == 0 || strcmp(pins[i].name, device->coil[k].b) == 0) {
                coil_pin = 1;
            }
        }
        if (use[i].entries > 1 && !(device->info.latching && !contact_pin)) {
            kind_error(ctx, "pin \"%s\" is in two entries", pins[i].name);
            return NULL;
        }
        if (use[i].entries == 0) {
            if (pins[i].role == PIN_INPUT || pins[i].role == PIN_INOUT) {
                kind_error(ctx, "pin \"%s\" is in no entry", pins[i].name);
                return NULL;
            }
            if (pins[i].role != PIN_NC) {
                kind_error(ctx, "relay pin \"%s\" must be nc", pins[i].name);
                return NULL;
            }
        } else if (coil_pin && !contact_pin && pins[i].role != PIN_INPUT) {
            kind_error(ctx, "relay pin \"%s\" must be input", pins[i].name);
            return NULL;
        } else if (contact_pin && pins[i].role != PIN_INOUT) {
            kind_error(ctx, "relay pin \"%s\" must be inout", pins[i].name);
            return NULL;
        }
    }
    for (k = 0; k < device->info.coils; k++) {
        if (first_pin(ctx, pins, pin_count, device->coil[k].a, tmp, &bound->coil_a[k]) != 0 ||
            first_pin(ctx, pins, pin_count, device->coil[k].b, tmp, &bound->coil_b[k]) != 0) {
            return NULL;
        }
        bound->polarized[k] = device->coil[k].polarized;
    }
    {
        size_t *common = kind_alloc(ctx, (pin_count + 1) * sizeof(*common));
        size_t *other = kind_alloc(ctx, (pin_count + 1) * sizeof(*other));

        if (common == NULL || other == NULL) {
            return NULL;
        }
        for (k = 0; k < device->info.poles; k++) {
            const PoleEntry *pole = &device->pole[k];
            size_t nc_count = kind_pins(ctx, pins, pin_count, pole->common, common, pin_count);
            size_t n;

            if (nc_count == 0) {
                return NULL;
            }
            for (i = 0; i < 2; i++) {
                const char *name = i == 0 ? pole->no : pole->nc;
                ContactSet *set = i == 0 ? &bound->no[k] : &bound->nc[k];

                if (name == NULL) {
                    if (contacts_new(ctx, set, 0) != 0) {
                        return NULL;
                    }
                    continue;
                }
                n = kind_pins(ctx, pins, pin_count, name, other, pin_count);
                if (n == 0 || contacts_new(ctx, set, nc_count * n) != 0) {
                    return NULL;
                }
                contacts_cross(set, common, nc_count, other, n);
            }
            bound->channel[k] = device->info.latching ? 0 : pole->coil - 1;
        }
        /* Every name an entry uses: its pins are one node. */
        for (i = 0; i < pin_count; i++) {
            size_t j;
            size_t n;

            if (use[i].entries == 0) {
                continue;
            }
            for (j = 0; j < i && strcmp(pins[j].name, pins[i].name) != 0; j++) {
            }
            if (j < i) {
                continue;                               /* not the first pin of its name */
            }
            n = kind_pins(ctx, pins, pin_count, pins[i].name, other, pin_count);
            contacts_within(&bound->ties, other, n);
        }
    }
    return bound;
}

/* A number above 0 and at most max, scaled by `unit` and rounded (at least 1); 0 when absent. */
static int scaled(KindContext *ctx, const cJSON *json, const char *key, int required, double max,
                  double unit, uint64_t *out) {
    const cJSON *item = kind_member(ctx, json, key, required);

    *out = 0;
    if (item == NULL) {
        return required ? -1 : 0;
    }
    if (!cJSON_IsNumber(item) || !(item->valuedouble > 0) || item->valuedouble > max) {
        return kind_error(ctx, "\"%s\" must be above 0 and at most %g", key, max);
    }
    *out = (uint64_t)llround(item->valuedouble * unit);
    if (*out == 0) {
        *out = 1;
    }
    return 0;
}

static int relay_load_part(KindContext *ctx, const void *raw, const void *bound, const cJSON *json,
                           Part *out) {
    const RelayDevice *device = raw;
    PartRelay *info = kind_alloc(ctx, sizeof(*info));
    int solid = strcmp(device->info.type, "solid-state") == 0;
    uint64_t v;

    (void)bound;
    if (info == NULL) {
        return -1;
    }
    *info = device->info;
    if (solid) {
        if (cJSON_GetObjectItemCaseSensitive(json, "coilVolts") != NULL) {
            return kind_error(ctx, "\"coilVolts\" is not a solid-state relay key");
        }
        if (cJSON_GetObjectItemCaseSensitive(json, "coilOhms") != NULL) {
            return kind_error(ctx, "\"coilOhms\" is not a solid-state relay key");
        }
    } else if (cJSON_GetObjectItemCaseSensitive(json, "onOhms") != NULL) {
        return kind_error(ctx, "\"onOhms\" is only for a solid-state relay");
    }
    if (!solid) {
        if (scaled(ctx, json, "coilVolts", 1, 1000, 1000.0, &v) != 0) {
            return -1;
        }
        info->coil_mv = (uint32_t)v;
        if (scaled(ctx, json, "coilOhms", 0, 10000000, 1000.0, &info->coil_mohm) != 0) {
            return -1;
        }
    }
    if (scaled(ctx, json, "contactAmps", 1, 1000, 1000.0, &v) != 0) {
        return -1;
    }
    info->contact_ma = (uint32_t)v;
    if (scaled(ctx, json, "contactVolts", 0, 100000, 1000.0, &v) != 0) {
        return -1;
    }
    info->contact_mv = (uint32_t)v;
    if (scaled(ctx, json, "operateMs", 0, 10000, 1000.0, &v) != 0) {
        return -1;
    }
    info->operate_us = (uint32_t)v;
    if (scaled(ctx, json, "releaseMs", 0, 10000, 1000.0, &v) != 0) {
        return -1;
    }
    info->release_us = (uint32_t)v;
    if (solid && scaled(ctx, json, "onOhms", 0, 100000, 1000.0, &info->on_mohm) != 0) {
        return -1;
    }
    out->relay = info;
    return 0;
}

/* The relay at rest, for the voltage check (§ 3.3): every pole's common-nc
   contacts, then the ties. */
int relay_fixed_contact(const Part *part, size_t k, size_t *a, size_t *b) {
    const RelayPins *pins;
    size_t pole;

    if (part == NULL || part->kind != PART_RELAY || part->relay == NULL) {
        return 0;
    }
    pins = part_internal(part)->bound;
    for (pole = 0; pole < part->relay->poles; pole++) {
        if (k < pins->nc[pole].count) {
            return contacts_at(&pins->nc[pole], k, a, b);
        }
        k -= pins->nc[pole].count;
    }
    return contacts_at(&pins->ties, k, a, b);
}

/* ---- Model --------------------------------------------------------------- */

typedef struct {
    const Part *part;
    const RelayPins *pins;
    unsigned char armature[MAX_COILS];
    Light light[MAX_COILS];
} RelayState;

static size_t relay_pin_count(const void *config) {
    return ((const Part *)config)->pin_count;
}

static void *relay_create(const void *config) {
    RelayState *state = logic_calloc(1, sizeof(*state));

    if (state != NULL) {
        state->part = config;
        state->pins = part_internal(config)->bound;
    }
    return state;
}

static void relay_destroy(void *state) {
    logic_free(state);
}

/* Moves every pole of a channel to the position: the open contacts first. */
static void move_poles(Sim *sim, SimInstance *instance, const RelayState *state, size_t channel, int operated) {
    const RelayPins *pins = state->pins;
    size_t pole;
    int pass;

    for (pass = 0; pass < 2; pass++) {
        for (pole = 0; pole < state->part->relay->poles; pole++) {
            if (pins->channel[pole] != channel) {
                continue;
            }
            if (pass == 0) {
                contacts_set(sim, instance, operated ? &pins->nc[pole] : &pins->no[pole], 0);
            } else {
                contacts_set(sim, instance, operated ? &pins->no[pole] : &pins->nc[pole], 1);
            }
        }
    }
}

static void relay_start(Sim *sim, SimInstance *instance, void *raw) {
    RelayState *state = raw;
    const RelayPins *pins = state->pins;
    size_t i;

    for (i = 0; i < state->part->relay->channels; i++) {
        state->armature[i] = 0;
        move_poles(sim, instance, state, i, 0);
        lights_set(&state->light[i], 0, 0, sim_now(sim));
        sim_schedule_timer(sim, instance, (uint32_t)i, RELAY_DELAY_PS);
    }
    contacts_set(sim, instance, &pins->ties, 1);
}

/* 1 when the coil is driven forward, -1 when driven backward, else 0. */
static int coil_drive(Sim *sim, SimInstance *instance, const RelayPins *pins, size_t coil) {
    LogicValue a = sim_read_pin(sim, instance, pins->coil_a[coil]);
    LogicValue b = sim_read_pin(sim, instance, pins->coil_b[coil]);

    if (a == LOGIC_1 && b == LOGIC_0) {
        return 1;
    }
    if (a == LOGIC_0 && b == LOGIC_1 && !pins->polarized[coil]) {
        return -1;
    }
    return 0;
}

static void relay_pin_changed(Sim *sim, SimInstance *instance, void *raw, size_t pin,
                              LogicValue old_value, LogicValue new_value) {
    RelayState *state = raw;
    const PartRelay *info = state->part->relay;
    size_t coil;

    (void)old_value;
    (void)new_value;
    for (coil = 0; coil < info->coils; coil++) {
        if (state->pins->coil_a[coil] == pin || state->pins->coil_b[coil] == pin) {
            sim_schedule_timer(sim, instance, info->latching ? 0 : (uint32_t)coil, RELAY_DELAY_PS);
        }
    }
}

static void relay_timer(Sim *sim, SimInstance *instance, void *raw, uint32_t timer_id) {
    RelayState *state = raw;
    const PartRelay *info = state->part->relay;
    const RelayPins *pins = state->pins;
    size_t channel = timer_id;
    int want;

    if (channel >= info->channels) {
        return;
    }
    want = state->armature[channel];
    if (!info->latching) {
        want = coil_drive(sim, instance, pins, channel) != 0;
    } else if (info->coils == 1) {
        int d = coil_drive(sim, instance, pins, 0);

        want = d > 0 ? 1 : d < 0 ? 0 : want;
    } else {
        int set = coil_drive(sim, instance, pins, 0) != 0;
        int reset = coil_drive(sim, instance, pins, 1) != 0;

        want = set && !reset ? 1 : reset && !set ? 0 : want;
    }
    if (want == state->armature[channel]) {
        return;
    }
    state->armature[channel] = (unsigned char)want;
    move_poles(sim, instance, state, channel, want);
    lights_set(&state->light[channel], 0, want, sim_now(sim));
}

static size_t relay_lights(const Sim *sim, const SimInstance *instance, const void *raw, SimTime since,
                           unsigned char *out, size_t cap) {
    const RelayState *state = raw;
    size_t i;
    size_t n = 0;

    (void)sim;
    (void)instance;
    for (i = 0; i < state->part->relay->channels; i++) {
        n += lights_read(&state->light[i], 1, since, out != NULL && n < cap ? out + n : NULL, n < cap ? cap - n : 0);
    }
    return n;
}

static size_t relay_light_time(const Sim *sim, const SimInstance *instance, const void *raw,
                               uint64_t *out, size_t cap) {
    const RelayState *state = raw;
    size_t i;
    size_t n = 0;

    (void)instance;
    for (i = 0; i < state->part->relay->channels; i++) {
        n += lights_time(&state->light[i], 1, sim_now(sim), out != NULL && n < cap ? out + n : NULL,
                         n < cap ? cap - n : 0);
    }
    return n;
}

static const SimComponentType RELAY_MODEL = {
    .name = "relay",
    .pin_count = relay_pin_count,
    .create = relay_create,
    .destroy = relay_destroy,
    .start = relay_start,
    .pin_changed = relay_pin_changed,
    .timer = relay_timer,
    .lights = relay_lights,
    .light_time = relay_light_time,
};

const PartKindOps RELAY_KIND = {
    PART_RELAY, "relay", &RELAY_MODEL, DEVICE_KEYS, PART_KEYS,
    relay_load_device, relay_bind_pinout, relay_load_part
};
