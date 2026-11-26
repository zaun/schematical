/*
 * dekatron: cold-cathode decade counting tubes (GC10B, GS10C, OG-4, Z504S,
 * ...). One anode, ten cathode positions in a ring, and guides between the
 * cathodes. The glow rests on one cathode and walks round the ring when the
 * guides are pulsed low. The model drives nothing; its ten lights are the
 * ten cathode positions (a light is on while the glow rests on that cathode).
 *
 * The glow exists while the anode reads 1 and strikes at the zero cathode
 * (position 0); an anode that stops reading 1 puts it out. A counter is reset
 * by switching its anode off and on again.
 *
 * Two-guide tubes ("drive": "double"). The ring is, in order: cathode k,
 * guide 1, guide 2, cathode k+1, ... (30 electrodes). A guide that falls to 0
 * pulls the glow onto its electrode when that sits next to the glow. When
 * both guides are back at 1 (or Z) a glow on a guide settles on the cathode
 * next to that guide: guide 1's is the one before it, guide 2's the one after
 * it. So the usual two overlapping pulses count one step forward (guide 1
 * low, guide 2 low, guide 1 high, guide 2 high) and the same two pulses the
 * other way round count backward; a single guide pulse, or two pulses that
 * do not overlap, count nothing.
 *
 * Single-pulse tubes ("drive": "single"). The ring is cathode k, guide k,
 * cathode k+1, ... (20 electrodes) and only the first guide is driven: a
 * 0 pulls the glow from the cathode onto the guide and its release moves
 * the glow on to the next cathode. One pulse is one step forward. Further
 * guides of the tube are only biased and do nothing here.
 *
 * The voltages are facts for the voltage check (voltage.c): the anode's level
 * is carried to every cathode net, and an anode below the tube's "strike"
 * voltage (the least supply) is a problem (VOLTAGE_NIXIE_UNDER).
 */

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "kind.h"
#include "lights.h"
#include "model_pins.h"
#include "logic_alloc.h"

#define DK_POSITIONS 10
#define DK_MAX_GUIDES 4
#define DK_MAX_VOLTS 800.0
#define DK_MAX_MA 100.0
#define DK_MAX_RATE 10000000u

static const char *const DEVICE_KEYS[] = {
    "anode", "cathodes", "guides", "drive", "color", "size", "view", "strike", "sustain",
    "current", "rate", NULL
};

static const char *const VIEWS[] = {"side", "end", NULL};

typedef struct {
    const char *name;
    unsigned positions;     /* bit per position 0-9 */
} CathodePin;

typedef struct {
    PartDekatron template;  /* everything except the pin lists */
    const char *anode;
    CathodePin *cathodes;
    size_t cathode_count;
    const char *const *guides;
    size_t guide_count;
} DekatronDevice;

typedef struct {
    PartDekatron dekatron;
    int double_drive;
} DekatronPins;

static int load_cathodes(KindContext *ctx, const cJSON *json, DekatronDevice *device) {
    const cJSON *cathodes = kind_member(ctx, json, "cathodes", 1);
    const cJSON *item;
    unsigned seen = 0;
    size_t n = 0;

    if (cathodes == NULL) {
        return -1;
    }
    if (!cJSON_IsObject(cathodes) || cathodes->child == NULL) {
        return kind_error(ctx, "\"cathodes\" must be a non-empty object");
    }
    if (kind_check_keys(ctx, cathodes, NULL, NULL) != 0) {
        return -1;
    }
    device->cathodes = kind_alloc(ctx, (size_t)cJSON_GetArraySize(cathodes) * sizeof(CathodePin));
    if (device->cathodes == NULL) {
        return -1;
    }
    cJSON_ArrayForEach(item, cathodes) {
        const cJSON *position;

        kind_push(ctx, "cathodes.%s", item->string);
        device->cathodes[n].name = kind_strdup(ctx, item->string);
        if (device->cathodes[n].name == NULL) {
            return -1;
        }
        if (!cJSON_IsArray(item) || cJSON_GetArraySize(item) == 0) {
            return kind_error(ctx, "a cathode pin lists its positions, an array of 0 to 9");
        }
        cJSON_ArrayForEach(position, item) {
            unsigned bit;

            if (!cJSON_IsNumber(position) || position->valuedouble < 0 ||
                position->valuedouble > DK_POSITIONS - 1 ||
                position->valuedouble != (double)(unsigned)position->valuedouble) {
                return kind_error(ctx, "a position is a whole number from 0 to 9");
            }
            bit = 1u << (unsigned)position->valuedouble;
            if (seen & bit) {
                return kind_error(ctx, "position %u is on two cathodes", (unsigned)position->valuedouble);
            }
            seen |= bit;
            device->cathodes[n].positions |= bit;
        }
        kind_pop(ctx);
        n++;
    }
    if (seen != (1u << DK_POSITIONS) - 1) {
        return kind_error(ctx, "\"cathodes\" must cover the positions 0 to 9 once each");
    }
    device->cathode_count = n;
    return 0;
}

/* "current": [min, max] milliamps of anode current, 0 < min <= max. */
static int load_current(KindContext *ctx, const cJSON *json, PartDekatron *d) {
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
    if (!(low > 0) || !(high <= DK_MAX_MA) || low > high) {
        return kind_error(ctx, "\"current\": 0 < min <= max <= %g", DK_MAX_MA);
    }
    d->current_min_ua = (uint32_t)llround(low * 1000);
    d->current_max_ua = (uint32_t)llround(high * 1000);
    return 0;
}

static void *dekatron_load_device(KindContext *ctx, const cJSON *json) {
    DekatronDevice *device = kind_alloc(ctx, sizeof(*device));
    PartDekatron *d;
    const char *drive;
    uint64_t rate;
    size_t i;
    size_t j;

    if (device == NULL) {
        return NULL;
    }
    d = &device->template;
    if (kind_string(ctx, json, "anode", &device->anode) != 0 ||
        load_cathodes(ctx, json, device) != 0 ||
        kind_names(ctx, json, "guides", &device->guides, &device->guide_count) != 0 ||
        kind_string(ctx, json, "drive", &drive) != 0 ||
        kind_string(ctx, json, "color", &d->color) != 0 ||
        kind_string(ctx, json, "size", &d->size) != 0 ||
        kind_string(ctx, json, "view", &d->view) != 0) {
        return NULL;
    }
    if (strcmp(drive, "double") == 0) {
        d->two_guide = 1;
    } else if (strcmp(drive, "single") == 0) {
        d->two_guide = 0;
    } else {
        kind_error(ctx, "\"drive\" must be \"double\" or \"single\"");
        return NULL;
    }
    if (device->guide_count == 0 || device->guide_count > DK_MAX_GUIDES ||
        (d->two_guide && device->guide_count != 2)) {
        kind_error(ctx, d->two_guide ? "a \"double\" tube has exactly 2 \"guides\""
                                     : "\"guides\" must name 1 to %d pins", DK_MAX_GUIDES);
        return NULL;
    }
    for (i = 0; VIEWS[i] != NULL && strcmp(VIEWS[i], d->view) != 0; i++) {
    }
    if (VIEWS[i] == NULL) {
        kind_error(ctx, "\"view\" must be \"side\" or \"end\"");
        return NULL;
    }
    d->view = VIEWS[i];
    for (i = 0; i < device->guide_count; i++) {
        if (strcmp(device->guides[i], device->anode) == 0) {
            kind_error(ctx, "pin \"%s\" is the anode and a guide", device->anode);
            return NULL;
        }
        for (j = 0; j < device->cathode_count; j++) {
            if (strcmp(device->guides[i], device->cathodes[j].name) == 0) {
                kind_error(ctx, "pin \"%s\" is a guide and a cathode", device->guides[i]);
                return NULL;
            }
        }
        for (j = 0; j < i; j++) {
            if (strcmp(device->guides[i], device->guides[j]) == 0) {
                kind_error(ctx, "guide \"%s\" is used twice", device->guides[i]);
                return NULL;
            }
        }
    }
    for (i = 0; i < device->cathode_count; i++) {
        if (strcmp(device->cathodes[i].name, device->anode) == 0) {
            kind_error(ctx, "pin \"%s\" is the anode and a cathode", device->anode);
            return NULL;
        }
    }
    if (kind_volts(ctx, json, "strike", DK_MAX_VOLTS, &d->strike_mv) != 0 ||
        kind_volts(ctx, json, "sustain", DK_MAX_VOLTS, &d->sustain_mv) != 0) {
        return NULL;
    }
    if (d->sustain_mv == 0 || d->sustain_mv > d->strike_mv) {
        kind_error(ctx, "0 < \"sustain\" <= \"strike\"");
        return NULL;
    }
    if (load_current(ctx, json, d) != 0 ||
        kind_uint(ctx, json, "rate", DK_MAX_RATE, &rate) != 0) {
        return NULL;
    }
    if (rate == 0) {
        kind_error(ctx, "\"rate\" must be at least 1");
        return NULL;
    }
    d->rate_hz = (uint32_t)rate;
    d->guide_count = device->guide_count;
    d->cathode_pin_count = device->cathode_count;
    return device;
}

static void *dekatron_bind_pinout(KindContext *ctx, const void *raw, const PartPin *pins,
                                  size_t pin_count) {
    const DekatronDevice *device = raw;
    DekatronPins *bound = kind_alloc(ctx, sizeof(*bound));
    size_t *cathode_pins;
    size_t *guide_pins;
    size_t i;
    unsigned p;

    if (bound == NULL) {
        return NULL;
    }
    bound->dekatron = device->template;
    bound->double_drive = device->template.two_guide;
    cathode_pins = kind_alloc(ctx, device->cathode_count * sizeof(*cathode_pins));
    guide_pins = kind_alloc(ctx, device->guide_count * sizeof(*guide_pins));
    if (cathode_pins == NULL || guide_pins == NULL) {
        return NULL;
    }
    kind_push(ctx, "anode");
    if (kind_pin(ctx, pins, pin_count, device->anode, &bound->dekatron.anode_pin) != 0) {
        return NULL;
    }
    kind_pop(ctx);
    for (i = 0; i < device->cathode_count; i++) {
        kind_push(ctx, "cathodes.%s", device->cathodes[i].name);
        if (kind_pin(ctx, pins, pin_count, device->cathodes[i].name, &cathode_pins[i]) != 0) {
            return NULL;
        }
        for (p = 0; p < DK_POSITIONS; p++) {
            if (device->cathodes[i].positions & (1u << p)) {
                bound->dekatron.position_pin[p] = cathode_pins[i];
            }
        }
        kind_pop(ctx);
    }
    for (i = 0; i < device->guide_count; i++) {
        kind_push(ctx, "guides[%zu]", i);
        if (kind_pin(ctx, pins, pin_count, device->guides[i], &guide_pins[i]) != 0) {
            return NULL;
        }
        kind_pop(ctx);
    }
    bound->dekatron.cathode_pins = cathode_pins;
    bound->dekatron.guide_pins = guide_pins;
    return bound;
}

static int dekatron_load_part(KindContext *ctx, const void *device, const void *bound,
                              const cJSON *json, Part *out) {
    (void)ctx;
    (void)device;
    (void)json;
    out->dekatron = &((const DekatronPins *)bound)->dekatron;
    return 0;
}

int dekatron_link(const Part *part, size_t k, size_t *anode, size_t *cathode) {
    const DekatronPins *pins;

    if (part == NULL || part->kind != PART_DEKATRON) {
        return 0;
    }
    pins = part_internal(part)->bound;
    if (k >= pins->dekatron.cathode_pin_count) {
        return 0;
    }
    *anode = pins->dekatron.anode_pin;
    *cathode = pins->dekatron.cathode_pins[k];
    return 1;
}

uint32_t dekatron_anode_min_mv(const Part *part, size_t pin) {
    const DekatronPins *pins;

    if (part == NULL || part->kind != PART_DEKATRON) {
        return 0;
    }
    pins = part_internal(part)->bound;
    return pin == pins->dekatron.anode_pin ? pins->dekatron.strike_mv : 0;
}

/* ---- Model --------------------------------------------------------------- */

typedef struct {
    const DekatronPins *pins;
    int glow;                       /* electrode the glow is on; -1 = no glow */
    Light lights[DK_POSITIONS];
} DekatronState;

static size_t dekatron_pin_count(const void *config) {
    return ((const Part *)config)->pin_count;
}

static void *dekatron_create(const void *config) {
    DekatronState *state = logic_calloc(1, sizeof(*state));

    if (state != NULL) {
        state->pins = part_internal(config)->bound;
        state->glow = -1;
    }
    return state;
}

static void dekatron_destroy(void *state) {
    logic_free(state);
}

/* The electrodes round the ring: a two-guide tube has 3 per position (cathode,
   guide 1, guide 2), a single-pulse tube 2 (cathode, guide). */
static int electrodes(const DekatronState *state) {
    return DK_POSITIONS * (state->pins->double_drive ? 3 : 2);
}

static int is_low(const Sim *sim, const SimInstance *instance, size_t pin) {
    return sim_read_pin(sim, instance, pin) == LOGIC_0;
}

static void publish(Sim *sim, DekatronState *state) {
    SimTime now = sim_now(sim);
    int stride = state->pins->double_drive ? 3 : 2;
    int p;

    for (p = 0; p < DK_POSITIONS; p++) {
        lights_set(state->lights, (size_t)p,
                   state->glow >= 0 && state->glow % stride == 0 && state->glow / stride == p, now);
    }
}

/* A glow on a guide electrode settles on the cathode beside it once the guides
   no longer hold it. */
static void settle(const Sim *sim, const SimInstance *instance, DekatronState *state) {
    const PartDekatron *d = &state->pins->dekatron;
    int n = electrodes(state);

    if (state->glow < 0) {
        return;
    }
    if (d->two_guide) {
        int kind = state->glow % 3;

        if (kind != 0 && !is_low(sim, instance, d->guide_pins[0]) &&
            !is_low(sim, instance, d->guide_pins[1])) {
            /* Guide 1 sits after its cathode, guide 2 before the next one. */
            state->glow = kind == 1 ? state->glow - 1 : (state->glow + 1) % n;
        }
    } else if (state->glow % 2 == 1 && !is_low(sim, instance, d->guide_pins[0])) {
        state->glow = (state->glow + 1) % n;
    }
}

/* Guide `index` fell: it pulls the glow onto its own electrode when that is
   next to the glow. */
static void pull(DekatronState *state, size_t index) {
    int n = electrodes(state);

    if (state->glow < 0) {
        return;
    }
    if (state->pins->double_drive) {
        int target = (int)index + 1;     /* guide 1 is electrode kind 1, guide 2 kind 2 */

        if ((state->glow + 1) % 3 == target) {
            state->glow = (state->glow + 1) % n;
        } else if ((state->glow + n - 1) % 3 == target) {
            state->glow = (state->glow + n - 1) % n;
        }
    } else if (index == 0 && state->glow % 2 == 0) {
        state->glow = state->glow + 1;
    }
}

static void dekatron_update(Sim *sim, SimInstance *instance, DekatronState *state) {
    const PartDekatron *d = &state->pins->dekatron;

    if (sim_read_pin(sim, instance, d->anode_pin) != LOGIC_1) {
        state->glow = -1;
    } else if (state->glow < 0) {
        state->glow = 0;
    }
    settle(sim, instance, state);
    publish(sim, state);
}

static void dekatron_start(Sim *sim, SimInstance *instance, void *raw) {
    dekatron_update(sim, instance, raw);
}

static void dekatron_pin_changed(Sim *sim, SimInstance *instance, void *raw, size_t pin,
                                 LogicValue old_value, LogicValue new_value) {
    DekatronState *state = raw;
    const PartDekatron *d = &state->pins->dekatron;
    size_t i;

    (void)old_value;
    if (new_value == LOGIC_0 && pin != d->anode_pin) {
        for (i = 0; i < d->guide_count && i < (d->two_guide ? 2u : 1u); i++) {
            if (d->guide_pins[i] == pin && old_value != LOGIC_0) {
                pull(state, i);
            }
        }
    }
    dekatron_update(sim, instance, state);
}

static size_t dekatron_lights(const Sim *sim, const SimInstance *instance, const void *raw,
                              SimTime since, unsigned char *out, size_t cap) {
    const DekatronState *state = raw;

    (void)sim;
    (void)instance;
    return lights_read(state->lights, DK_POSITIONS, since, out, cap);
}

static size_t dekatron_light_time(const Sim *sim, const SimInstance *instance, const void *raw,
                                  uint64_t *out, size_t cap) {
    const DekatronState *state = raw;

    (void)instance;
    return lights_time(state->lights, DK_POSITIONS, sim_now(sim), out, cap);
}

static const SimComponentType DEKATRON_MODEL = {
    "dekatron", dekatron_pin_count, dekatron_create, dekatron_destroy, dekatron_start,
    dekatron_pin_changed, NULL, dekatron_lights, NULL, NULL, NULL, dekatron_light_time
};

const PartKindOps DEKATRON_KIND = {
    PART_DEKATRON, "dekatron", &DEKATRON_MODEL, DEVICE_KEYS, NULL,
    dekatron_load_device, dekatron_bind_pinout, dekatron_load_part
};
