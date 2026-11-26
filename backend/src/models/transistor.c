/*
 * transistor (plan-40-F): bipolar transistors and MOSFETs, one or more
 * elements per package. Each element is a digital switch: its control pin
 * (base, gate) opens and closes an engine contact between its channel pins
 * (collector-emitter, drain-source). An NPN or N-channel element conducts
 * while its control reads 1, a PNP or P-channel one while it reads 0; any
 * other reading (0 / 1 the other way, X, Z) leaves the channel open. The
 * channel follows the control 1 ns later (TRANSISTOR_DELAY_PS, from a timer
 * that re-reads the pin), so a pulse shorter than that never moves it and a
 * gate tied to its own drain toggles every 1 ns instead of looping at one
 * time. A conducting channel is a strong two-way contact, as a closed switch:
 * a strong 1 and a strong 0 across it is a short. No analog behaviour.
 *
 * Pins that carry the same name (a tab and its lead) are one node: tied by
 * permanently closed contacts, made when the simulation starts.
 */

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "kind.h"
#include "model_pins.h"
#include "logic_alloc.h"

#define TRANSISTOR_DELAY_PS 1000u

static const char *const DEVICE_KEYS[] = {"elements", NULL};
static const char *const PART_KEYS[] = {"vceo", "ic", "pd", "hfe", "r1", "r2", "vds", "id",
                                        "rdson", "vgsth", "vgs", "logicLevel", NULL};

enum { CONTROL, CHANNEL_A, CHANNEL_B };

typedef struct {
    const char *type;      /* "npn", "pnp", "nmos", "pmos" */
    int mosfet;
    int on_level;          /* the control reading that closes the channel */
    int darlington;
    const char *names[3];  /* control, channel a, channel b */
} TransistorElement;

typedef struct {
    size_t count;
    TransistorElement *elements;
    int mixed;
    int darlington;
} TransistorDevice;

/* A pinout's pins, as indices into Part.pins. channel: every (a, b) pin pair
   of each element (a name on several pins gives several pairs); ties: every
   pair of same-named channel pins. */
typedef struct {
    size_t count;
    const TransistorDevice *device;
    size_t *control;       /* per element */
    size_t *first_a;       /* per element: its first channel a and b pin */
    size_t *first_b;
    size_t *channel_from;  /* per element: [channel_from[k], channel_from[k + 1]) */
    size_t *channel_a;
    size_t *channel_b;
    size_t tie_count;
    size_t *tie_a;
    size_t *tie_b;
} TransistorPins;

static const char *const BJT_NAMES[3] = {"base", "collector", "emitter"};
static const char *const FET_NAMES[3] = {"gate", "drain", "source"};

static void *transistor_load_device(KindContext *ctx, const cJSON *json) {
    TransistorDevice *device = kind_alloc(ctx, sizeof(*device));
    const cJSON *elements;
    const cJSON *item;
    size_t count = 0;

    if (device == NULL) {
        return NULL;
    }
    elements = kind_member(ctx, json, "elements", 1);
    if (elements == NULL) {
        return NULL;
    }
    if (!cJSON_IsArray(elements) || cJSON_GetArraySize(elements) == 0) {
        kind_error(ctx, "\"elements\" must be a non-empty array of transistors");
        return NULL;
    }
    device->elements = kind_alloc(ctx, (size_t)cJSON_GetArraySize(elements) * sizeof(*device->elements));
    if (device->elements == NULL) {
        return NULL;
    }
    cJSON_ArrayForEach(item, elements) {
        static const char *const BJT_KEYS[] = {"type", "base", "collector", "emitter", "darlington", NULL};
        static const char *const FET_KEYS[] = {"type", "gate", "drain", "source", NULL};
        TransistorElement *e = &device->elements[count];
        const char *const *names;
        const char *const *other;
        size_t n;

        kind_push(ctx, "elements[%zu]", count);
        if (!cJSON_IsObject(item)) {
            kind_error(ctx, "an element is {\"type\": …, and its pin names}");
            return NULL;
        }
        if (kind_string(ctx, item, "type", &e->type) != 0) {
            return NULL;
        }
        if (strcmp(e->type, "npn") == 0 || strcmp(e->type, "pnp") == 0) {
            e->mosfet = 0;
            names = BJT_NAMES;
            other = FET_NAMES;
        } else if (strcmp(e->type, "nmos") == 0 || strcmp(e->type, "pmos") == 0) {
            e->mosfet = 1;
            names = FET_NAMES;
            other = BJT_NAMES;
        } else {
            kind_error(ctx, "unknown transistor type \"%s\" (npn, pnp, nmos, pmos)", e->type);
            return NULL;
        }
        e->on_level = e->type[0] == 'n' ? 1 : 0;
        for (n = 0; n < 3; n++) {
            if (cJSON_GetObjectItemCaseSensitive(item, other[n]) != NULL) {
                kind_error(ctx, "\"%s\" is not a %s key", other[n], e->mosfet ? "mosfet" : "bjt");
                return NULL;
            }
        }
        if (e->mosfet && cJSON_GetObjectItemCaseSensitive(item, "darlington") != NULL) {
            kind_error(ctx, "\"darlington\" is not a mosfet key");
            return NULL;
        }
        if (kind_check_keys(ctx, item, e->mosfet ? FET_KEYS : BJT_KEYS, NULL) != 0) {
            return NULL;
        }
        for (n = 0; n < 3; n++) {
            if (kind_string(ctx, item, names[n], &e->names[n]) != 0) {
                return NULL;
            }
        }
        if (strcmp(e->names[0], e->names[1]) == 0 || strcmp(e->names[0], e->names[2]) == 0 ||
            strcmp(e->names[1], e->names[2]) == 0) {
            kind_error(ctx, "an element's three pins must have different names");
            return NULL;
        }
        e->darlington = 0;
        if (!e->mosfet && cJSON_GetObjectItemCaseSensitive(item, "darlington") != NULL &&
            kind_bool(ctx, item, "darlington", &e->darlington) != 0) {
            return NULL;
        }
        if (count > 0 && strcmp(e->type, device->elements[0].type) != 0) {
            device->mixed = 1;
        }
        device->darlington |= e->darlington;
        kind_pop(ctx);
        count++;
    }
    device->count = count;
    return device;
}

static void *transistor_bind_pinout(KindContext *ctx, const void *raw, const PartPin *pins,
                                    size_t pin_count) {
    const TransistorDevice *device = raw;
    TransistorPins *bound = kind_alloc(ctx, sizeof(*bound));
    size_t *as = kind_alloc(ctx, (pin_count + 1) * sizeof(*as));
    size_t *bs = kind_alloc(ctx, (pin_count + 1) * sizeof(*bs));
    unsigned char *used = kind_alloc(ctx, pin_count + 1);
    size_t cap = device->count * pin_count * pin_count + 1;
    size_t tie_cap = pin_count * pin_count + 1;
    size_t i;
    size_t k;

    if (bound == NULL || as == NULL || bs == NULL || used == NULL) {
        return NULL;
    }
    bound->count = device->count;
    bound->device = device;
    bound->control = kind_alloc(ctx, (device->count + 1) * sizeof(*bound->control));
    bound->first_a = kind_alloc(ctx, (device->count + 1) * sizeof(*bound->first_a));
    bound->first_b = kind_alloc(ctx, (device->count + 1) * sizeof(*bound->first_b));
    bound->channel_from = kind_alloc(ctx, (device->count + 1) * sizeof(*bound->channel_from));
    bound->channel_a = kind_alloc(ctx, cap * sizeof(*bound->channel_a));
    bound->channel_b = kind_alloc(ctx, cap * sizeof(*bound->channel_b));
    bound->tie_a = kind_alloc(ctx, tie_cap * sizeof(*bound->tie_a));
    bound->tie_b = kind_alloc(ctx, tie_cap * sizeof(*bound->tie_b));
    if (bound->control == NULL || bound->first_a == NULL || bound->first_b == NULL ||
        bound->channel_from == NULL || bound->channel_a == NULL || bound->channel_b == NULL ||
        bound->tie_a == NULL || bound->tie_b == NULL) {
        return NULL;
    }
    for (k = 0; k < device->count; k++) {
        const TransistorElement *e = &device->elements[k];
        size_t control[2];
        size_t nc = kind_pins(ctx, pins, pin_count, e->names[CONTROL], control, 2);
        size_t na;
        size_t nb;
        size_t x;

        if (nc == 0) {
            return NULL;
        }
        if (nc > 1) {
            kind_error(ctx, "transistor pin \"%s\" must be on one pin", e->names[CONTROL]);
            return NULL;
        }
        if (pins[control[0]].role != PIN_INPUT) {
            kind_error(ctx, "transistor pin \"%s\" must be input", e->names[CONTROL]);
            return NULL;
        }
        na = kind_pins(ctx, pins, pin_count, e->names[CHANNEL_A], as, pin_count);
        if (na == 0) {
            return NULL;
        }
        nb = kind_pins(ctx, pins, pin_count, e->names[CHANNEL_B], bs, pin_count);
        if (nb == 0) {
            return NULL;
        }
        for (x = 0; x < na + nb; x++) {
            size_t p = x < na ? as[x] : bs[x - na];

            if (pins[p].role != PIN_INOUT) {
                kind_error(ctx, "transistor pin \"%s\" must be inout", pins[p].name);
                return NULL;
            }
            used[p] = 1;
        }
        used[control[0]] = 1;
        bound->control[k] = control[0];
        bound->first_a[k] = as[0];
        bound->first_b[k] = bs[0];
    }
    /* The channel pairs, element by element: every pin of a with every pin of b. */
    {
        size_t n = 0;

        for (k = 0; k < device->count; k++) {
            const TransistorElement *e = &device->elements[k];
            size_t na = kind_pins(ctx, pins, pin_count, e->names[CHANNEL_A], as, pin_count);
            size_t nb = kind_pins(ctx, pins, pin_count, e->names[CHANNEL_B], bs, pin_count);
            size_t x;
            size_t y;

            bound->channel_from[k] = n;
            for (x = 0; x < na; x++) {
                for (y = 0; y < nb; y++) {
                    bound->channel_a[n] = as[x];
                    bound->channel_b[n] = bs[y];
                    n++;
                }
            }
        }
        bound->channel_from[device->count] = n;
    }
    /* Same-named pins: every pair tied, so any wired subset of them is one node. */
    for (i = 0; i < pin_count; i++) {
        size_t j;

        if (pins[i].role != PIN_INOUT) {
            continue;
        }
        for (j = i + 1; j < pin_count; j++) {
            if (pins[j].role == PIN_INOUT && strcmp(pins[j].name, pins[i].name) == 0) {
                bound->tie_a[bound->tie_count] = i;
                bound->tie_b[bound->tie_count++] = j;
            }
        }
    }
    for (i = 0; i < pin_count; i++) {
        if (pins[i].role != PIN_NC && pins[i].role != PIN_ABSENT && !used[i]) {
            kind_error(ctx, "pin \"%s\" is in no element", pins[i].name);
            return NULL;
        }
    }
    return bound;
}

/* An optional part number in (0, max] (zero_ok: [0, max]); *mill gets
   llround(value * 1000), 0 when not given. */
static int optional_milli(KindContext *ctx, const cJSON *json, const char *key, double max,
                          int zero_ok, uint64_t *mill) {
    const cJSON *item = kind_member(ctx, json, key, 0);

    *mill = 0;
    if (item == NULL) {
        return 0;
    }
    if (!cJSON_IsNumber(item) || !(zero_ok ? item->valuedouble >= 0 : item->valuedouble > 0) ||
        item->valuedouble > max) {
        return kind_error(ctx, "\"%s\" must be %s 0 and at most %g", key,
                          zero_ok ? "at least" : "above", max);
    }
    *mill = (uint64_t)llround(item->valuedouble * 1000);
    return 0;
}

static int transistor_load_part(KindContext *ctx, const void *raw, const void *bound,
                                const cJSON *json, Part *out) {
    static const char *const BJT_ONLY[] = {"vceo", "ic", "hfe", "r1", "r2", NULL};
    static const char *const FET_ONLY[] = {"vds", "id", "rdson", "vgsth", "vgs", "logicLevel", NULL};
    const TransistorDevice *device = raw;
    PartTransistor *info = kind_alloc(ctx, sizeof(*info));
    int mosfet = device->elements[0].mosfet;
    const char *const *wrong = mosfet ? BJT_ONLY : FET_ONLY;
    uint64_t vmax = 0, imax = 0, power = 0, hfe = 0, r1 = 0, r2 = 0, rdson = 0, vgsth = 0, vgs = 0;
    size_t n;

    (void)bound;
    if (info == NULL) {
        return -1;
    }
    for (n = 0; wrong[n] != NULL; n++) {
        if (cJSON_GetObjectItemCaseSensitive(json, wrong[n]) != NULL) {
            return kind_error(ctx, "\"%s\" is only for a %s", wrong[n], mosfet ? "bjt" : "mosfet");
        }
    }
    if (optional_milli(ctx, json, mosfet ? "vds" : "vceo", 10000, 0, &vmax) != 0 ||
        optional_milli(ctx, json, mosfet ? "id" : "ic", 1000, 0, &imax) != 0 ||
        optional_milli(ctx, json, "pd", 10000, 0, &power) != 0) {
        return -1;
    }
    if (mosfet) {
        if (optional_milli(ctx, json, "rdson", 1e6, 0, &rdson) != 0 ||
            optional_milli(ctx, json, "vgsth", 100, 0, &vgsth) != 0 ||
            optional_milli(ctx, json, "vgs", 1000, 0, &vgs) != 0) {
            return -1;
        }
        if (cJSON_GetObjectItemCaseSensitive(json, "logicLevel") != NULL) {
            int logic = 0;

            if (kind_bool(ctx, json, "logicLevel", &logic) != 0) {
                return -1;
            }
            info->logic_level = (uint8_t)logic;
        }
    } else {
        const cJSON *gain = kind_member(ctx, json, "hfe", 0);

        if (gain != NULL) {
            if (!cJSON_IsNumber(gain) || !(gain->valuedouble > 0) || gain->valuedouble > 100000) {
                return kind_error(ctx, "\"hfe\" must be above 0 and at most 100000");
            }
            hfe = (uint64_t)llround(gain->valuedouble);
        }
        if (optional_milli(ctx, json, "r1", 1e9, 1, &r1) != 0 ||
            optional_milli(ctx, json, "r2", 1e9, 1, &r2) != 0) {
            return -1;
        }
    }
    info->type = device->elements[0].type;
    info->element_count = device->count;
    info->mixed = (uint8_t)device->mixed;
    info->darlington = (uint8_t)device->darlington;
    info->mosfet = (uint8_t)mosfet;
    info->vmax_mv = (uint32_t)vmax;
    info->imax_ma = (uint32_t)imax;
    info->power_mw = (uint32_t)power;
    info->hfe = (uint32_t)hfe;
    info->vgsth_mv = (uint32_t)vgsth;
    info->vgs_mv = (uint32_t)vgs;
    info->r1_mohm = r1;
    info->r2_mohm = r2;
    info->rdson_mohm = rdson;
    out->transistor = info;
    return 0;
}

int transistor_element(const Part *part, size_t k, const char **type, size_t *control,
                       size_t *a, size_t *b) {
    const TransistorPins *bound;

    if (part == NULL || part->kind != PART_TRANSISTOR) {
        return 0;
    }
    bound = part_internal(part)->bound;
    if (k >= bound->count) {
        return 0;
    }
    if (type != NULL) {
        *type = bound->device->elements[k].type;
    }
    if (control != NULL) {
        *control = bound->control[k];
    }
    if (a != NULL) {
        *a = bound->first_a[k];
    }
    if (b != NULL) {
        *b = bound->first_b[k];
    }
    return 1;
}

uint32_t transistor_pin_limit_mv(const Part *part, size_t pin) {
    const TransistorPins *bound;
    size_t k;

    if (part == NULL || part->kind != PART_TRANSISTOR || part->transistor == NULL ||
        pin >= part->pin_count) {
        return 0;
    }
    bound = part_internal(part)->bound;
    for (k = 0; k < bound->count; k++) {
        if (bound->control[k] == pin) {
            return bound->device->elements[k].mosfet ? part->transistor->vgs_mv : 0;
        }
    }
    if (part->pins[pin].role == PIN_INOUT) {
        return part->transistor->vmax_mv;
    }
    return 0;
}

/* ---- Model --------------------------------------------------------------- */

typedef struct {
    const TransistorPins *pins;
    unsigned char on[];
} TransistorState;

static size_t transistor_pin_count(const void *config) {
    return ((const Part *)config)->pin_count;
}

static void *transistor_create(const void *config) {
    const TransistorPins *pins = part_internal(config)->bound;
    TransistorState *state = logic_calloc(1, sizeof(*state) + pins->count + 1);

    if (state != NULL) {
        state->pins = pins;
    }
    return state;
}

static void transistor_destroy(void *state) {
    logic_free(state);
}

static void transistor_start(Sim *sim, SimInstance *instance, void *raw) {
    TransistorState *state = raw;
    size_t i;

    for (i = 0; i < state->pins->tie_count; i++) {
        sim_set_contact(sim, instance, state->pins->tie_a[i], state->pins->tie_b[i], 1);
    }
    for (i = 0; i < state->pins->count; i++) {
        sim_schedule_timer(sim, instance, (uint32_t)i, TRANSISTOR_DELAY_PS);
    }
}

static void transistor_pin_changed(Sim *sim, SimInstance *instance, void *raw, size_t pin,
                                   LogicValue old_value, LogicValue new_value) {
    TransistorState *state = raw;
    size_t k;

    (void)old_value;
    (void)new_value;
    for (k = 0; k < state->pins->count; k++) {
        if (state->pins->control[k] == pin) {
            sim_schedule_timer(sim, instance, (uint32_t)k, TRANSISTOR_DELAY_PS);
        }
    }
}

static void transistor_timer(Sim *sim, SimInstance *instance, void *raw, uint32_t timer_id) {
    TransistorState *state = raw;
    const TransistorPins *pins = state->pins;
    size_t k = timer_id;   /* < count: start and pin_changed schedule only an element k < count */
    LogicValue v;
    int want;
    size_t i;

    v = sim_read_pin(sim, instance, pins->control[k]);
    want = pins->device->elements[k].on_level ? v == LOGIC_1 : v == LOGIC_0;
    if (want == state->on[k]) {
        return;
    }
    state->on[k] = (unsigned char)want;
    for (i = pins->channel_from[k]; i < pins->channel_from[k + 1]; i++) {
        sim_set_contact(sim, instance, pins->channel_a[i], pins->channel_b[i], want);
    }
}

static const SimComponentType TRANSISTOR_MODEL = {
    .name = "transistor",
    .pin_count = transistor_pin_count,
    .create = transistor_create,
    .destroy = transistor_destroy,
    .start = transistor_start,
    .pin_changed = transistor_pin_changed,
    .timer = transistor_timer,
};

const PartKindOps TRANSISTOR_KIND = {
    PART_TRANSISTOR, "transistor", &TRANSISTOR_MODEL, DEVICE_KEYS, PART_KEYS,
    transistor_load_device, transistor_bind_pinout, transistor_load_part
};
