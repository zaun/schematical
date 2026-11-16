/*
 * nixie: cold-cathode numeric indicator tubes (IN-14, IN-18, ...). A tube
 * is one anode and one cathode per glyph (a numeral, a symbol, a decimal
 * point). It is simulated like a set of LEDs that share the anode: a glyph
 * glows while the anode reads 1 and its cathode reads 0. The model drives
 * nothing. Lights are tracked with lights.c, so a multiplexed tube scanned
 * during the app's last step counts as lit.
 *
 * A tube with two anode pins (one name on two pins) has them joined inside
 * the part (a closed contact); the anode reads as the first that is not Z.
 *
 * The voltages are facts for the voltage check (voltage.c): the anode's
 * level is carried to every cathode net, and an anode below the tube's
 * "strike" voltage is a problem (VOLTAGE_NIXIE_UNDER).
 */

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "kind.h"
#include "lights.h"
#include "model_pins.h"

#define NIXIE_MAX_CATHODES 16
#define NIXIE_MAX_ANODES 2
#define NIXIE_MAX_GLYPH 8      /* bytes of UTF-8 */
#define NIXIE_MAX_VOLTS 500.0
#define NIXIE_MAX_MA 100.0

static const char *const DEVICE_KEYS[] = {
    "anode", "cathodes", "color", "size", "view", "strike", "sustain", "current", NULL
};

static const char *const VIEWS[] = {"side", "end", NULL};

typedef struct {
    PartNixie nixie_template;   /* everything except the pin lists */
    const char *anode;
    const char *cathode_names[NIXIE_MAX_CATHODES];
    const char **glyphs;
    size_t cathode_count;
} NixieDevice;

typedef struct {
    PartNixie nixie;
    size_t anodes[NIXIE_MAX_ANODES];
    size_t anode_count;
    size_t cathodes[NIXIE_MAX_CATHODES];
    size_t cathode_count;
} NixiePins;

static int load_cathodes(KindContext *ctx, const cJSON *json, NixieDevice *device) {
    static const char *const KEYS[] = {"pin", "glyph", NULL};
    const cJSON *cathodes = kind_member(ctx, json, "cathodes", 1);
    const cJSON *item;
    size_t n = 0;

    if (cathodes == NULL) {
        return -1;
    }
    if (!cJSON_IsArray(cathodes) || cJSON_GetArraySize(cathodes) == 0 ||
        cJSON_GetArraySize(cathodes) > NIXIE_MAX_CATHODES) {
        return kind_error(ctx, "\"cathodes\" must be an array of 1 to %d cathodes",
                          NIXIE_MAX_CATHODES);
    }
    device->glyphs = kind_alloc(ctx, (size_t)cJSON_GetArraySize(cathodes) * sizeof(*device->glyphs));
    if (device->glyphs == NULL) {
        return -1;
    }
    cJSON_ArrayForEach(item, cathodes) {
        const char *glyph;
        size_t i;

        kind_push(ctx, "cathodes[%zu]", n);
        if (kind_check_keys(ctx, item, KEYS, NULL) != 0 ||
            kind_string(ctx, item, "pin", &device->cathode_names[n]) != 0 ||
            kind_string(ctx, item, "glyph", &glyph) != 0) {
            return -1;
        }
        if (glyph[0] == '\0' || strlen(glyph) > NIXIE_MAX_GLYPH) {
            return kind_error(ctx, "\"glyph\" must be 1 to %d bytes", NIXIE_MAX_GLYPH);
        }
        for (i = 0; i < n; i++) {
            if (strcmp(device->glyphs[i], glyph) == 0) {
                return kind_error(ctx, "glyph \"%s\" is used twice", glyph);
            }
            if (strcmp(device->cathode_names[i], device->cathode_names[n]) == 0) {
                return kind_error(ctx, "pin \"%s\" is used twice", device->cathode_names[n]);
            }
        }
        device->glyphs[n] = glyph;
        kind_pop(ctx);
        n++;
    }
    device->cathode_count = n;
    return 0;
}

/* "current": [min, max] milliamps per cathode, 0 < min <= max. */
static int load_current(KindContext *ctx, const cJSON *json, PartNixie *nixie) {
    const cJSON *item = kind_member(ctx, json, "current", 1);
    double low;
    double high;

    if (item == NULL) {
        return -1;
    }
    if (!cJSON_IsArray(item) || cJSON_GetArraySize(item) != 2 ||
        !cJSON_IsNumber(cJSON_GetArrayItem(item, 0)) ||
        !cJSON_IsNumber(cJSON_GetArrayItem(item, 1))) {
        return kind_error(ctx, "\"current\" must be [min, max] in milliamps");
    }
    low = cJSON_GetArrayItem(item, 0)->valuedouble;
    high = cJSON_GetArrayItem(item, 1)->valuedouble;
    if (!(low > 0) || !(high <= NIXIE_MAX_MA) || low > high) {
        return kind_error(ctx, "\"current\": 0 < min <= max <= %g", NIXIE_MAX_MA);
    }
    nixie->current_min_ua = (uint32_t)llround(low * 1000);
    nixie->current_max_ua = (uint32_t)llround(high * 1000);
    return 0;
}

static void *nixie_load_device(KindContext *ctx, const cJSON *json) {
    NixieDevice *device = kind_alloc(ctx, sizeof(*device));
    PartNixie *n;
    size_t i;

    if (device == NULL) {
        return NULL;
    }
    n = &device->nixie_template;
    if (kind_string(ctx, json, "anode", &device->anode) != 0 ||
        load_cathodes(ctx, json, device) != 0 ||
        kind_string(ctx, json, "color", &n->color) != 0 ||
        kind_string(ctx, json, "size", &n->size) != 0 ||
        kind_string(ctx, json, "view", &n->view) != 0) {
        return NULL;
    }
    for (i = 0; VIEWS[i] != NULL && strcmp(VIEWS[i], n->view) != 0; i++) {
    }
    if (VIEWS[i] == NULL) {
        kind_error(ctx, "\"view\" must be \"side\" or \"end\"");
        return NULL;
    }
    n->view = VIEWS[i];
    for (i = 0; i < device->cathode_count; i++) {
        if (strcmp(device->cathode_names[i], device->anode) == 0) {
            kind_error(ctx, "pin \"%s\" is the anode and a cathode", device->anode);
            return NULL;
        }
    }
    if (kind_volts(ctx, json, "strike", NIXIE_MAX_VOLTS, &n->strike_mv) != 0 ||
        kind_volts(ctx, json, "sustain", NIXIE_MAX_VOLTS, &n->sustain_mv) != 0) {
        return NULL;
    }
    if (n->sustain_mv == 0 || n->sustain_mv > n->strike_mv) {
        kind_error(ctx, "0 < \"sustain\" <= \"strike\"");
        return NULL;
    }
    if (load_current(ctx, json, n) != 0) {
        return NULL;
    }
    n->glyphs = device->glyphs;
    n->glyph_count = device->cathode_count;
    return device;
}

static void *nixie_bind_pinout(KindContext *ctx, const void *raw, const PartPin *pins,
                               size_t pin_count) {
    const NixieDevice *device = raw;
    NixiePins *bound = kind_alloc(ctx, sizeof(*bound));
    size_t *cathode_pins;
    size_t *anode_pins;
    size_t i;

    if (bound == NULL) {
        return NULL;
    }
    kind_push(ctx, "anode");
    bound->anode_count = kind_pins(ctx, pins, pin_count, device->anode, bound->anodes,
                                   NIXIE_MAX_ANODES);
    if (bound->anode_count == 0) {
        return NULL;
    }
    kind_pop(ctx);
    for (i = 0; i < device->cathode_count; i++) {
        kind_push(ctx, "cathodes[%zu]", i);
        if (kind_pin(ctx, pins, pin_count, device->cathode_names[i], &bound->cathodes[i]) != 0) {
            return NULL;
        }
        kind_pop(ctx);
    }
    bound->cathode_count = device->cathode_count;

    cathode_pins = kind_alloc(ctx, bound->cathode_count * sizeof(*cathode_pins));
    anode_pins = kind_alloc(ctx, bound->anode_count * sizeof(*anode_pins));
    if (cathode_pins == NULL || anode_pins == NULL) {
        return NULL;
    }
    memcpy(cathode_pins, bound->cathodes, bound->cathode_count * sizeof(*cathode_pins));
    memcpy(anode_pins, bound->anodes, bound->anode_count * sizeof(*anode_pins));
    bound->nixie = device->nixie_template;
    bound->nixie.cathode_pins = cathode_pins;
    bound->nixie.anode_pins = anode_pins;
    bound->nixie.anode_pin_count = bound->anode_count;
    return bound;
}

static int nixie_load_part(KindContext *ctx, const void *device, const void *bound,
                           const cJSON *json, Part *out) {
    (void)ctx;
    (void)device;
    (void)json;
    out->nixie = &((const NixiePins *)bound)->nixie;
    return 0;
}

int nixie_link(const Part *part, size_t k, size_t *anode, size_t *cathode) {
    const NixiePins *pins;

    if (part == NULL || part->kind != PART_NIXIE) {
        return 0;
    }
    pins = part_internal(part)->bound;
    if (k >= pins->anode_count * pins->cathode_count) {
        return 0;
    }
    *anode = pins->anodes[k / pins->cathode_count];
    *cathode = pins->cathodes[k % pins->cathode_count];
    return 1;
}

uint32_t nixie_anode_min_mv(const Part *part, size_t pin) {
    const NixiePins *pins;
    size_t i;

    if (part == NULL || part->kind != PART_NIXIE) {
        return 0;
    }
    pins = part_internal(part)->bound;
    for (i = 0; i < pins->anode_count; i++) {
        if (pins->anodes[i] == pin) {
            return pins->nixie.strike_mv;
        }
    }
    return 0;
}

/* ---- Model --------------------------------------------------------------- */

typedef struct {
    const NixiePins *pins;
    Light lights[NIXIE_MAX_CATHODES];
} NixieState;

static size_t nixie_pin_count(const void *config) {
    return ((const Part *)config)->pin_count;
}

static void *nixie_create(const void *config) {
    NixieState *state = calloc(1, sizeof(*state));

    if (state != NULL) {
        state->pins = part_internal(config)->bound;
    }
    return state;
}

static void nixie_destroy(void *state) {
    free(state);
}

static void nixie_update(Sim *sim, SimInstance *instance, NixieState *state) {
    SimTime now = sim_now(sim);
    LogicValue anode = LOGIC_Z;
    size_t i;

    for (i = 0; i < state->pins->anode_count && anode == LOGIC_Z; i++) {
        anode = sim_read_pin(sim, instance, state->pins->anodes[i]);
    }
    for (i = 0; i < state->pins->cathode_count; i++) {
        int on = anode == LOGIC_1 &&
                 sim_read_pin(sim, instance, state->pins->cathodes[i]) == LOGIC_0;

        lights_set(state->lights, i, on, now);
    }
}

static void nixie_start(Sim *sim, SimInstance *instance, void *raw) {
    NixieState *state = raw;

    if (state->pins->anode_count == 2) {
        sim_set_contact(sim, instance, state->pins->anodes[0], state->pins->anodes[1], 1);
    }
    nixie_update(sim, instance, state);
}

static void nixie_pin_changed(Sim *sim, SimInstance *instance, void *state, size_t pin,
                              LogicValue old_value, LogicValue new_value) {
    (void)pin;
    (void)old_value;
    (void)new_value;
    nixie_update(sim, instance, state);
}

static size_t nixie_lights(const Sim *sim, const SimInstance *instance, const void *raw,
                           SimTime since, unsigned char *out, size_t cap) {
    const NixieState *state = raw;

    (void)sim;
    (void)instance;
    return lights_read(state->lights, state->pins->cathode_count, since, out, cap);
}

static size_t nixie_light_time(const Sim *sim, const SimInstance *instance, const void *raw,
                               uint64_t *out, size_t cap) {
    const NixieState *state = raw;

    (void)instance;
    return lights_time(state->lights, state->pins->cathode_count, sim_now(sim), out, cap);
}

static const SimComponentType NIXIE_MODEL = {
    "nixie", nixie_pin_count, nixie_create, nixie_destroy, nixie_start, nixie_pin_changed,
    NULL, nixie_lights, NULL, NULL, NULL, nixie_light_time
};

const PartKindOps NIXIE_KIND = {
    PART_NIXIE, "nixie", &NIXIE_MODEL, DEVICE_KEYS, NULL,
    nixie_load_device, nixie_bind_pinout, nixie_load_part
};
